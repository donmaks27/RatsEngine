#include <engine/render/glfw/window1_glfw_vulkan_service.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window1_glfw_vulkan_service)

    surface_backend_service* surface_backend_service::instance_allocate_vulkan()
    {
        return new window1_glfw_vulkan_service();
    }

    bool window1_glfw_vulkan_service::service_init(const render_api_service_create_info& createInfo)
    {
        return super_t::service_init(createInfo);
    }
    void window1_glfw_vulkan_service::service_clear()
    {
        super_t::service_clear();
    }
}
