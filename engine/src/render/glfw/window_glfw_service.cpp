#include <engine/render/glfw/window_glfw_service.h>

#include "engine/engine.h"

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window_glfw_service)

    namespace
    {
        void glfw_error_callback(const int error_code, const char* description)
        {
            log::error("[GLFW] Code {:X}: {}", error_code, description);
        }
    }

    bool window_glfw_service::service_init(const render_api_service_create_info& info)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            Log.fatal("Failed to initialize GLFW!");
            return false;
        }
        glfwWindowHint(GLFW_CLIENT_API, glfw_client_api());
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        return super_t::service_init(info);
    }
    void window_glfw_service::service_clear()
    {
        super_t::service_clear();
        glfwTerminate();
    }

    bool window_glfw_service::create_window_impl(const surface_id id, const window_create_info& info)
    {
        const auto window = glfwCreateWindow(
            static_cast<int>(info.size.x), static_cast<int>(info.size.y),
            "RatsEngine", nullptr, nullptr
        );
        if (window == nullptr)
        {
            Log.error("Failed to create GLFW window");
            return false;
        }
        m_windowDataGLFW[id] = { .window = window };

        if (!super_t::create_window_impl(id, info))
        {
            m_windowDataGLFW.erase(id);
            glfwDestroyWindow(window);
            return false;
        }
        return true;
    }
    void window_glfw_service::destroy_window_impl(const surface_id id)
    {
        super_t::destroy_window_impl(id);

        glfwDestroyWindow(m_windowDataGLFW[id].window);
        m_windowDataGLFW.erase(id);
    }

    bool window_glfw_service::should_close_main_window() const
    {
        return glfwWindowShouldClose(m_windowDataGLFW.begin()->second.window) != GLFW_FALSE;
    }

    GLFWwindow* window_glfw_service::glfw_window(const surface_id id) const
    {
        const auto iter = m_windowDataGLFW.find(id);
        return iter != m_windowDataGLFW.end() ? iter->second.window : nullptr;
    }

    void window_glfw_service::poll_window_events()
    {
        glfwPollEvents();

        const auto mainWindowId = main_window_id();
        for (const auto windowId : window_ids())
        {
            const auto window = glfw_window(windowId);
            if (glfwWindowShouldClose(window) == GLFW_TRUE)
            {
                if (windowId != mainWindowId)
                {
                    destroy_window(windowId);
                }
                else
                {
                    engine::instance().events().post<engine_shutdown_signal_event>({});
                }
            }
        }
    }
}
