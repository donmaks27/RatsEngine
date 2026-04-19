#include <engine/render/vulkan/surface_vulkan_service.h>

#include <engine/render/vulkan/render_vulkan_service.h>

namespace engine
{
	RATS_ENGINE_SERVICE_IMPL(surface_vulkan_service)

	surface_service* surface_service::instance_allocate_vulkan() { return new surface_vulkan_service(); }

	bool surface_vulkan_service::on_event(const event_info& event)
	{
		return event.dispatch<vulkan_device_created_event>([this] {
			return on_device_created();
		});
	}

	bool surface_vulkan_service::create_surface(const vulkan::context& ctx, const surface_id id, const surface_vulkan_create_info& info)
	{
		if (!super_t::create_surface(id, info))
		{
			return false;
		}
		m_surfaces[id] = { .surface = info.surface };
		if (!create_swapchain(ctx, id))
		{
			destroy_surface(id);
			return false;
		}
		return true;
	}
	void surface_vulkan_service::destroy_surface(const surface_id id)
	{
		const auto iter = m_surfaces.find(id);
		if (iter != m_surfaces.end())
		{
			const auto& ctx = render_vulkan_service::instance()->vk_ctx();
			auto& [surface, swapchain] = iter->second;
			swapchain.clear(ctx);
			ctx.i()->destroySurfaceKHR(surface);

			m_surfaces.erase(id);
		}

		super_t::destroy_surface(id);
	}

	bool surface_vulkan_service::create_swapchain(const vulkan::context& ctx, const surface_id id)
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
	bool surface_vulkan_service::on_device_created()
	{
		const auto& ctx = render_vulkan_service::instance()->vk_ctx();
		return std::ranges::all_of(surface_ids(), [this, &ctx](const surface_id id) {
			return create_swapchain(ctx, id);
		});
	}
}
