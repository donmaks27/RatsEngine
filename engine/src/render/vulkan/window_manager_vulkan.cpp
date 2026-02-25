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
        if ((ctx.i() != nullptr) && !create_surface_for_window(id))
        {
            log::fatal("[window_manager_vulkan::on_create_window] Failed to create surface for window {}", id);
            return false;
        }
        return true;
    }
    bool window_manager_vulkan::on_instance_created()
    {
        const auto windowManager = window_manager::instance();
        return std::ranges::all_of(windowManager->window_ids(), [this](const window_id& id) {
            if (!create_surface_for_window(id))
            {
                log::fatal("[window_manager_vulkan::on_instance_created] Failed to create surface for window {}", id);
                return false;
			}
            return true;
        });
    }
    bool window_manager_vulkan::create_surface_for_window(const window_id& id)
    {
		auto& windowData = m_windowDataVulkan[id];
        if (windowData.surface == nullptr)
        {
            const auto surface = create_surface(id);
            if (surface == nullptr)
            {
                return false;
			}
			windowData.surface = surface;
        }
        return true;
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

    vk::SurfaceKHR window_manager_vulkan::surface(const window_id& id) const
    {
        const auto iter = m_windowDataVulkan.find(id);
        return iter != m_windowDataVulkan.end() ? iter->second.surface : nullptr;
    }

    void window_manager_vulkan::clear_vulkan()
    {
        if (!m_windowDataVulkan.empty())
        {
            const auto& ctx = render_manager_vulkan::instance()->vk_ctx();
            for (const auto& [id, data] : m_windowDataVulkan)
            {
                ctx.i()->destroySurfaceKHR(data.surface);
            }
            m_windowDataVulkan.clear();
        }
    }
}
