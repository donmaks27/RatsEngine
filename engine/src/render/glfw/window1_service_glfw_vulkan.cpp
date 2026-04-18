#include <engine/render/glfw/window1_service_glfw_vulkan.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window1_service_glfw_vulkan)

    surface_backend_service* surface_backend_service::instance_allocate_vulkan()
    {
        return new window1_service_glfw_vulkan();
    }

    bool window1_service_glfw_vulkan::service_init(const render_api_service_create_info& createInfo)
    {
        return super_t::service_init(createInfo);
    }
    void window1_service_glfw_vulkan::service_clear()
    {
        super_t::service_clear();
    }
}
