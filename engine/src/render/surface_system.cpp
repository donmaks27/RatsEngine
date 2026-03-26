#include <engine/render/surface_system.h>

namespace engine
{
	const log::logger surface_system::Log = surface_system::logger();
	surface_system* surface_system::Instance = nullptr;

	surface_system* surface_system::instance_allocate(const instance_create_info& info)
	{
		surface_system* result = nullptr;
		switch (info.renderApi)
		{
		case render_api::vulkan: result = instance_allocate_vulkan(); break;
		default:;
		}
		if (result == nullptr)
		{
			Log.fatal("Render API '{}' is not implemented", info.renderApi);
		}
		return result;
	}

	surface_id surface_system::create_surface(const glm::uvec2& size)
	{
		static surface_id prevSurfaceId = invalid_surface_id;
		if (m_surfaces.size() >= std::numeric_limits<surface_id>::max())
		{
			return invalid_surface_id;
		}

		surface_id id = ++prevSurfaceId;
		while ((m_surfaces.count(id) != 0) || (id == invalid_surface_id))
		{
			id++;
		}
		m_surfaces[id] = { .size = size };
		return id;
	}
	void surface_system::clear_surface(const surface_id id)
	{
		m_surfaces.erase(id);
	}
}
