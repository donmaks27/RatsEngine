#include <engine/render/glfw/window_glfw_service.h>

#include <engine/engine.h>

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

    bool window_glfw_service::service_init()
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            Log.fatal("Failed to initialize GLFW!");
            return false;
        }
        glfwWindowHint(GLFW_CLIENT_API, glfw_client_api());
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        return super_t::service_init();
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

    GLFWwindow* window_glfw_service::glfw_window(const surface_id id) const
    {
        const auto iter = m_windowDataGLFW.find(id);
        return iter != m_windowDataGLFW.end() ? iter->second.window : nullptr;
    }

    void window_glfw_service::poll_events()
    {
        glfwPollEvents();

        using window_pair_t = decltype(m_windowDataGLFW)::value_type;
        static eastl::vector<surface_id> closingWindowIds;
        closingWindowIds.clear();
        std::ranges::copy(m_windowDataGLFW | std::views::filter([](const window_pair_t& data) {
            auto* window = data.second.window;
            return (window != nullptr) && (glfwWindowShouldClose(window) == GLFW_TRUE);
        }) | std::views::transform([](const window_pair_t& data) {
            return data.first;
        }), std::back_inserter(closingWindowIds));

        for (const auto windowId : closingWindowIds)
        {
            const auto window = glfw_window(windowId);
            if (glfwWindowShouldClose(window) == GLFW_TRUE)
            {
                if (windowId != primary_surface_id())
                {
                    destroy_window(windowId);
                }
                else
                {
                    engine::instance().signal_shutdown();
                }
            }
        }
    }
}
