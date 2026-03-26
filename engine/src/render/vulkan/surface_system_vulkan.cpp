#include <engine/render/vulkan/surface_system_vulkan.h>

#include <engine/render/vulkan/render_system_vulkan.h>

namespace engine
{
	const log::logger surface_system_vulkan::Log = surface_system_vulkan::logger();
	surface_system_vulkan* surface_system_vulkan::Instance = nullptr;

	bool surface_system_vulkan::system_init(const instance_create_info& info)
	{
		Instance = this;
		return super::system_init(info);
	}
	void surface_system_vulkan::system_clear()
	{
		const auto& ctx = render_system_vulkan::instance()->vk_ctx();
		std::ranges::for_each(surface_ids(), [this, &ctx](const surface_id id) {
			clear_surface(ctx, id);
		});
		m_surfaces.clear();

		super::system_clear();
		Instance = nullptr;
	}

	bool surface_system_vulkan::on_event(const utils::event_info& event)
	{
		return event.dispatch<vulkan_device_created_event>([this] {
			return on_device_created();
		});
	}

	surface_id surface_system_vulkan::create_surface(const vulkan::context& ctx, const vk::SurfaceKHR surface, const glm::uvec2& size)
	{
		const auto id = super::create_surface(size);
		if (id != invalid_surface_id)
		{
			m_surfaces[id] = { .surface = surface };
			if (!create_swapchain(ctx, id))
			{
				clear_surface(ctx, id);
				return invalid_surface_id;
			}
		}
		return id;
	}
	void surface_system_vulkan::clear_surface(const vulkan::context& ctx, const surface_id id)
	{
		const auto iter = m_surfaces.find(id);
		if (iter != m_surfaces.end())
		{
			auto& data = iter->second;
			data.swapchain.clear(ctx);
			ctx.i()->destroySurfaceKHR(data.surface);
		}
		m_surfaces.erase(id);

		super::clear_surface(id);
	}

	bool surface_system_vulkan::create_swapchain(const vulkan::context& ctx, const surface_id id)
	{
		if (ctx.d() == nullptr)
		{
			// Swapchain creation will be deferred until device creation
			return true;
		}

		auto& data = m_surfaces.at_key(id);
		if (!data.swapchain.init(ctx, { .surface = data.surface, .surfaceSize = surface_size(id) }))
		{
			Log.error("Failed to create swapchain for surface {}", id);
			return false;
		}
		return true;
	}
	bool surface_system_vulkan::on_device_created()
	{
		const auto& ctx = render_system_vulkan::instance()->vk_ctx();
		return std::ranges::all_of(surface_ids(), [this, &ctx](const surface_id id) {
			return create_swapchain(ctx, id);
		});
	}
}
