#include <engine/render/surface_service.h>

namespace engine
{
	RATS_ENGINE_SERVICE_IMPL(surface_service)
	RATS_ENGINE_SERVICE_IMPL(surface_backend_service)

#if !RATS_ENGINE_VULKAN_ENABLE
	surface_service* surface_service::instance_allocate_vulkan() { return nullptr; }
	surface_backend_service* surface_backend_service::instance_allocate_vulkan() { return nullptr; }
#endif

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

	void surface_backend_service::service_clear()
	{
		m_surfaceIdGenerator.reset();
	}
}
