#include <engine/render/vulkan/window_service_vulkan.h>

#include <engine/engine.h>
#include <engine/render/vulkan/render_service_vulkan.h>

namespace engine
{
    window_service_vulkan* window_service_vulkan::Instance = nullptr;

    bool window_service_vulkan::handle_event(const event_info& event)
    {
        return event.dispatch<vulkan_instance_created_event>([this] {
            return on_instance_created();
        }) && event.dispatch<vulkan_device_created_event>([this] {
            return on_device_created();
        });
    }

    bool window_service_vulkan::on_window_created(const window_id& id)
    {
		const auto& ctx = render_service_vulkan::instance()->vk_ctx();
        return create_surface(ctx, id) && create_swapchain(ctx, id);
    }
    void window_service_vulkan::on_window_destroying(const window_id& id)
    {
        const auto iter = m_windowDataVulkan.find(id);
        if (iter != m_windowDataVulkan.end())
        {
            render_service_vulkan::instance()->vk_ctx().i()->destroySurfaceKHR(iter->second.surface);
            m_windowDataVulkan.erase(iter);
        }
    }

    bool window_service_vulkan::on_instance_created()
    {
        const auto& ctx = render_service_vulkan::instance()->vk_ctx();
        return std::ranges::all_of(window_service::instance()->window_ids(), [this, &ctx](const window_id& id) {
            return create_surface(ctx, id);
        });
    }
    bool window_service_vulkan::on_device_created()
    {
        const auto& ctx = render_service_vulkan::instance()->vk_ctx();
        return std::ranges::all_of(window_service::instance()->window_ids(), [this, &ctx](const window_id& id) {
            return create_swapchain(ctx, id);
        });
    }

    bool window_service_vulkan::create_surface(const vulkan::context& ctx, const window_id& id)
    {
        if (ctx.i() == nullptr)
        {
			// Surface creation will be deferred until instance creation
            return true;
		}
        const auto surface = create_surface_impl(ctx, id);
        if (surface == nullptr)
        {
            Log.fatal("Failed to create surface for window {}!", id);
            return false;
        }
        m_windowDataVulkan[id].surface = surface;
		return true;
    }
    vk::SurfaceKHR window_service_vulkan::surface(const window_id& id) const
    {
        const auto iter = m_windowDataVulkan.find(id);
        return iter != m_windowDataVulkan.end() ? iter->second.surface : nullptr;
    }

    bool window_service_vulkan::create_swapchain(const vulkan::context& ctx, const window_id& id)
    {
        if (ctx.d() == nullptr)
        {
            // Swapchain creation will be deferred until device creation
            return true;
        }
        
		const auto size = window_service::instance()->window_size(id);
        if (!m_windowDataVulkan[id].swapchain.init(ctx, { .surface = surface(id), .surfaceSize = size }))
        {
            Log.fatal("Failed to create swapchain for window {}!", id);
            return false;
        }
        return true;
    }
	vulkan::swapchain* window_service_vulkan::swapchain(const window_id& id)
    {
        const auto iter = m_windowDataVulkan.find(id);
        return iter != m_windowDataVulkan.end() ? &iter->second.swapchain : nullptr;
    }

    void window_service_vulkan::clear_vulkan(const vulkan::context& ctx)
    {
        if (!m_windowDataVulkan.empty())
        {
            for (auto& [id, data] : m_windowDataVulkan)
            {
                data.swapchain.clear(ctx);
                ctx.i()->destroySurfaceKHR(data.surface);
            }
            m_windowDataVulkan.clear();
        }
    }
}
