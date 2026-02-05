#include <engine/render/glfw/window_manager_glfw.h>

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
    bool window_manager_glfw::should_close_window(const window_id& id) const
    {
        const auto iter = m_windowDataGLFW.find(id);
        return (iter != m_windowDataGLFW.end()) && glfwWindowShouldClose(iter->second);
    }

    void window_manager_glfw::on_frame_end()
    {
        glfwPollEvents();
    }

    bool window_manager_glfw::init(const create_info& info)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            log::fatal("[window_manager_glfw::init] Failed to initialize GLFW!");
            return false;
        }
        log::info("[window_manager_glfw::init] GLFW initialized");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return super::init(info);
    }

    void window_manager_glfw::clear()
    {
        clear_GLFW();
        super::clear();
    }

    void window_manager_glfw::clear_GLFW()
    {
        for (const auto& [id, window] : m_windowDataGLFW)
        {
            glfwDestroyWindow(window);
        }
        m_windowDataGLFW.clear();
        glfwTerminate();
    }

    bool window_manager_glfw::create_window_impl(const window_id& id, const window_create_info& info)
    {
        const auto window = glfwCreateWindow(800, 600, "RatsEngine", nullptr, nullptr);
        if (window == nullptr)
        {
            log::error("[window_manager_glfw::create_window_impl] Failed to create window");
            return false;
        }
        m_windowDataGLFW.emplace(id, window);
        return true;
    }
}