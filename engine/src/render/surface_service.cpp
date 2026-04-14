#include <engine/render/surface_service.h>

namespace engine
{
	surface_service* surface_service::Instance = nullptr;
	surface_backend_service* surface_backend_service::Instance = nullptr;

	surface_id surface_service::create_surface(const surface_create_info& info)
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
	void surface_service::clear_surface(const surface_id id)
	{
		m_surfaces.erase(id);
	}
}
