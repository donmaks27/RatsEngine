#include <engine/render/vulkan/window_manager_vulkan.h>

#include <engine/render/vulkan/render_manager_vulkan.h>

namespace engine
{
    window_manager_vulkan* window_manager_vulkan::s_instanceVulkan = nullptr;

    void window_manager_vulkan::on_init()
    {
        s_instanceVulkan = this;
    }
    void window_manager_vulkan::on_clear()
    {
        s_instanceVulkan = nullptr;
    }

    bool window_manager_vulkan::on_window_created(const window_id& id)
    {
		const auto& ctx = render_manager_vulkan::instance()->vk_ctx();
        return create_surface(ctx, id) && create_swapchain(ctx, id);
    }
    void window_manager_vulkan::on_window_destroying(const window_id& id)
    {
        const auto iter = m_windowDataVulkan.find(id);
        if (iter != m_windowDataVulkan.end())
        {
            render_manager_vulkan::instance()->vk_ctx().i()->destroySurfaceKHR(iter->second.surface);
            m_windowDataVulkan.erase(iter);
        }
    }

    bool window_manager_vulkan::on_instance_created(const vulkan::context& ctx)
    {
        const auto windowManager = window_manager::instance();
        return std::ranges::all_of(windowManager->window_ids(), [this, &ctx](const window_id& id) {
            return create_surface(ctx, id);
        });
    }
    bool window_manager_vulkan::on_device_created(const vulkan::context& ctx)
    {
        const auto windowManager = window_manager::instance();
        return std::ranges::all_of(windowManager->window_ids(), [this, &ctx](const window_id& id) {
            return create_swapchain(ctx, id);
        });
    }

    bool window_manager_vulkan::create_surface(const vulkan::context& ctx, const window_id& id)
    {
        if (ctx.i() == nullptr)
        {
			// Surface creation will be deferred until instance creation
            return true;
		}
        const auto surface = create_surface_impl(ctx, id);
        if (surface == nullptr)
        {
            log::fatal("[window_manager_vulkan::create_surface] Failed to create surface for window {}!", id);
            return false;
        }
        m_windowDataVulkan[id].surface = surface;
		return true;
    }
    vk::SurfaceKHR window_manager_vulkan::surface(const window_id& id) const
    {
        const auto iter = m_windowDataVulkan.find(id);
        return iter != m_windowDataVulkan.end() ? iter->second.surface : nullptr;
    }

    bool window_manager_vulkan::create_swapchain(const vulkan::context& ctx, const window_id& id)
    {
        if (ctx.d() == nullptr)
        {
            // Swapchain creation will be deferred until device creation
            return true;
        }
        
		const auto size = window_manager::instance()->window_size(id);
        if (!m_windowDataVulkan[id].swapchain.init(ctx, { .surface = surface(id), .surfaceSize = size }))
        {
            log::fatal("[window_manager_vulkan::create_swapchain] Failed to create swapchain for window {}!", id);
            return false;
        }
        return true;
    }
	const vulkan::swapchain* window_manager_vulkan::swapchain(const window_id& id) const
    {
        const auto iter = m_windowDataVulkan.find(id);
        return iter != m_windowDataVulkan.end() ? &iter->second.swapchain : nullptr;
    }

    void window_manager_vulkan::clear_vulkan(const vulkan::context& ctx)
    {
        if (!m_windowDataVulkan.empty())
        {
            for (auto& [id, data] : m_windowDataVulkan)
            {
                data.swapchain.clear();
                ctx.i()->destroySurfaceKHR(data.surface);
            }
            m_windowDataVulkan.clear();
        }
    }
}
