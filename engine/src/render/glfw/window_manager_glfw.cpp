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
    bool window_manager_glfw::shouldCloseMainWindow() const
    {
        return glfwWindowShouldClose(m_mainWindow) == GLFW_TRUE;
    }

    void window_manager_glfw::on_frame_end()
    {
        glfwPollEvents();
    }

    bool window_manager_glfw::init(const create_info& info)
    {
        if (!super::init(info))
        {
            return false;
        }

        glfwSetErrorCallback(glfw_error_callback);

        if (!glfwInit())
        {
            log::fatal("[window_manager_glfw::init_window_manager] Failed to initialize GLFW!");
            return false;
        }
        log::info("[window_manager_glfw::init_window_manager] GLFW initialized");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_mainWindow = glfwCreateWindow(static_cast<int>(800), static_cast<int>(600), "MainWindow", nullptr, nullptr);
        if (m_mainWindow == nullptr)
        {
            log::fatal("[window_manager_glfw::init_window_manager] Failed to create main GLFW window");
            return false;
        }
        log::info("[window_manager_glfw::init_window_manager] Created main GLFW window");
        return true;
    }

    void window_manager_glfw::clear()
    {
        clear_GLFW();
        super::clear();
    }

    void window_manager_glfw::clear_GLFW()
    {
        if (m_mainWindow != nullptr)
        {
            glfwDestroyWindow(m_mainWindow);
            m_mainWindow = nullptr;
            log::info("[window_manager_glfw::clear_GLFW] Main GLFW window destroyed");
        }

        glfwTerminate();
        log::info("[window_manager_glfw::clear_GLFW] GLFW terminated");
    }
}