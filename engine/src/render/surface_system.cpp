#include <engine/render/surface_system.h>

namespace engine
{
	surface_system* surface_system::Instance = nullptr;
	surface_backend_system* surface_backend_system::Instance = nullptr;

	surface_id surface_system::create_surface(const surface_create_info& info)
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
		m_surfaces[id] = { .size = info.size };
		return id;
	}
	void surface_system::clear_surface(const surface_id id)
	{
		m_surfaces.erase(id);
	}
}
