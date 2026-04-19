#include <engine/render/glfw/window1_glfw_vulkan_service.h>

#include <engine/render/vulkan/render_vulkan_service.h>
#include <engine/render/vulkan/surface_vulkan_service.h>

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

    bool window1_glfw_vulkan_service::create_window_impl(const surface_id id, const window_create_info& info)
    {
        if (!super_t::create_window_impl(id, info))
        {
            return false;
        }

        const auto& ctx = render_vulkan_service::instance()->vk_ctx();

        VkSurfaceKHR vulkanSurface = nullptr;
        const auto result = static_cast<vk::Result>(
            glfwCreateWindowSurface(*ctx.i(), glfw_window(id), nullptr, &vulkanSurface)
        );
        if (result != vk::Result::eSuccess)
        {
            Log.error("Failed to create window surface: {}", result);
            destroy_window_impl(id);
            return false;
        }
        if (!surface_vulkan_service::instance()->create_surface(ctx, id, { info.size, vulkanSurface }))
        {
            destroy_window_impl(id);
            return false;
        }
        return true;
    }
}
