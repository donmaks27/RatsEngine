#include <engine/render/vulkan/surface_service_vulkan.h>

#include <engine/render/vulkan/render_service_vulkan.h>

namespace engine
{
	RATS_ENGINE_SERVICE_IMPL(surface_service_vulkan)

	surface_service* surface_service::instance_allocate_vulkan() { return new surface_service_vulkan(); }

	void surface_service_vulkan::service_clear()
	{
		const auto& ctx = render_service_vulkan::instance()->vk_ctx();
		std::ranges::for_each(surface_ids(), [this, &ctx](const surface_id id) {
			clear_surface(ctx, id);
		});
		m_surfaces.clear();

		super_t::service_clear();
	}

	bool surface_service_vulkan::on_event(const event_info& event)
	{
		return event.dispatch<vulkan_device_created_event>([this] {
			return on_device_created();
		});
	}

	surface_id surface_service_vulkan::create_surface(const vulkan::context& ctx, const vulkan_surface_create_info& info)
	{
		const auto id = super_t::create_surface(info);
		if (id != invalid_surface_id)
		{
			m_surfaces[id] = { .surface = info.surface };
			if (!create_swapchain(ctx, id))
			{
				clear_surface(ctx, id);
				return invalid_surface_id;
			}
		}
		return id;
	}
	void surface_service_vulkan::clear_surface(const vulkan::context& ctx, const surface_id id)
	{
		const auto iter = m_surfaces.find(id);
		if (iter != m_surfaces.end())
		{
			auto& data = iter->second;
			data.swapchain.clear(ctx);
			ctx.i()->destroySurfaceKHR(data.surface);
		}
		m_surfaces.erase(id);

		super_t::clear_surface(id);
	}

	bool surface_service_vulkan::create_swapchain(const vulkan::context& ctx, const surface_id id)
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
	bool surface_service_vulkan::on_device_created()
	{
		const auto& ctx = render_service_vulkan::instance()->vk_ctx();
		return std::ranges::all_of(surface_ids(), [this, &ctx](const surface_id id) {
			return create_swapchain(ctx, id);
		});
	}
}
