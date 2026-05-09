#include <engine/render/glfw/window_glfw_vulkan_service.h>

#include <engine/render/vulkan/render_vulkan_service.h>
#include <engine/render/vulkan/surface_vulkan_service.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window_glfw_vulkan_service)

    surface_backend_service* allocate_surface_backend_service_vulkan()
    {
        return new window_glfw_vulkan_service();
    }

    eastl::vector<const char*> surface_vulkan_service::required_instance_extensions()
    {
        uint32_t extensionCount = 0;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return { extensions, extensions + extensionCount };
    }

    bool window_glfw_vulkan_service::service_init()
    {
        return super_t::service_init();
    }
    void window_glfw_vulkan_service::service_clear()
    {
        super_t::service_clear();
    }

    bool window_glfw_vulkan_service::on_event(const event_info& event)
    {
        return event.dispatch<vulkan_instance_created_event>([this] {
            return on_instance_created();
        });
    }

    bool window_glfw_vulkan_service::create_window_impl(const surface_id id, const window_create_info& info)
    {
        if (!super_t::create_window_impl(id, info))
        {
            return false;
        }

        const auto& ctx = render_vulkan_service::instance().vk_ctx();
        if (ctx.i() == nullptr)
        {
            // Surface creation will be deferred until instance creation
            return true;
        }
        if (!create_surface(ctx, id))
        {
            destroy_window_impl(id);
            return false;
        }
        return true;
    }

    bool window_glfw_vulkan_service::on_instance_created() const
    {
        const auto& ctx = render_vulkan_service::instance().vk_ctx();
        return std::ranges::all_of(window_ids(), [this, &ctx](const surface_id id) {
            return create_surface(ctx, id);
        });
    }
    bool window_glfw_vulkan_service::create_surface(const vulkan::context& ctx, const surface_id id) const
    {
        VkSurfaceKHR vulkanSurface = nullptr;
        const auto result = static_cast<vk::Result>(
            glfwCreateWindowSurface(*ctx.i(), glfw_window(id), nullptr, &vulkanSurface)
        );
        if (result != vk::Result::eSuccess)
        {
            Log.error("Failed to create window surface: {}", result);
            return false;
        }
        return surface_vulkan_service::instance().create_surface(ctx, id, { window_size(id), vulkanSurface });
    }
}
