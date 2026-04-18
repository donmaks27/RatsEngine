#include <engine/render/glfw/window1_service_glfw.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window1_service_glfw)

    namespace
    {
        void glfw_error_callback(const int error_code, const char* description)
        {
            log::error("[GLFW] Code {:X}: {}", error_code, description);
        }
    }

    bool window1_service_glfw::service_init(const render_api_service_create_info& info)
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            Log.fatal("Failed to initialize GLFW!");
            return false;
        }
        glfwWindowHint(GLFW_CLIENT_API, glfw_client_api());
        return super_t::service_init(info);
    }
    void window1_service_glfw::service_clear()
    {
        super_t::service_clear();
        glfwTerminate();
    }
}
