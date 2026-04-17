#include <engine/render/glfw/window_service_glfw.h>

#include <GLFW/glfw3.h>

namespace
{
    void glfw_error_callback(const int error_code, const char* description)
    {
        engine::log::error("[GLFW] Code {:X}: {}", error_code, description);
    }
}

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window_service_glfw)

    bool window_service_glfw::should_close_window(const window_id& id) const
    {
        const auto iter = m_windowDataGLFW.find(id);
        return (iter != m_windowDataGLFW.end()) && glfwWindowShouldClose(iter->second);
    }

    void window_service_glfw::on_frame_end()
    {
        glfwPollEvents();
    }

    bool window_service_glfw::service_init(const service_create_info_t& info)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            Log.fatal("Failed to initialize GLFW!");
            return false;
        }
        Log.info("GLFW initialized");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return super_t::service_init(info);
    }

    void window_service_glfw::service_clear()
    {
        clear_GLFW();
        super_t::service_clear();
    }

    void window_service_glfw::clear_GLFW()
    {
        for (const auto& [id, window] : m_windowDataGLFW)
        {
            glfwDestroyWindow(window);
        }
        m_windowDataGLFW.clear();
        glfwTerminate();
    }

    bool window_service_glfw::create_window_impl(const window_id& id, const window_create_info& info)
    {
        const auto window = glfwCreateWindow(static_cast<int>(info.size.x), static_cast<int>(info.size.y), "RatsEngine", nullptr, nullptr);
        if (window == nullptr)
        {
            Log.error("Failed to create GLFW window");
            return false;
        }
        Log.log("GLFW window created successfully");
        m_windowDataGLFW.emplace(id, window);
        return true;
    }

    void window_service_glfw::destroy_window_impl(const window_id& id)
    {
        const auto iter = m_windowDataGLFW.find(id);
        glfwDestroyWindow(iter->second);
        m_windowDataGLFW.erase(iter);
    }
}
