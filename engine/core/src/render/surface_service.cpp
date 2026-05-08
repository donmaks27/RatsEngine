#include <engine/render/surface_service.h>

namespace engine
{
	RATS_ENGINE_SERVICE_IMPL(surface_service)
	RATS_ENGINE_SERVICE_IMPL(surface_backend_service)

#if !RATS_ENGINE_VULKAN_ENABLE
	surface_service* allocate_surface_service_vulkan() { return nullptr; }
	surface_backend_service* allocate_surface_backend_service() { return nullptr; }
	surface_service* surface_service::instance_allocate_vulkan() { return nullptr; }
	surface_backend_service* surface_backend_service::instance_allocate_vulkan() { return nullptr; }
#endif

	surface_service* allocate_surface_service(const render_api renderApi)
	{
		switch (renderApi)
		{
		case render_api::vulkan: return allocate_surface_service_vulkan();
		default: ;
		}
		return nullptr;
	}
	surface_backend_service* allocate_surface_backend_service(const render_api renderApi)
	{
		switch (renderApi)
		{
		case render_api::vulkan: return allocate_surface_backend_service_vulkan();
		default: ;
		}
		return nullptr;
	}

	bool surface_service::service_init()
	{
		return true;
	}
	void surface_service::service_clear()
	{
		clear_surfaces();
	}

	bool surface_service::create_surface(const surface_id id, const surface_create_info& info)
	{
		m_surfaces[id] = { .size = info.size };
		return true;
	}
	void surface_service::destroy_surface(const surface_id id)
	{
		m_surfaces.erase(id);
	}

	void surface_service::clear_surfaces()
	{
		while (!m_surfaces.empty())
		{
			destroy_surface(m_surfaces.back().first);
		}
	}

	bool surface_backend_service::service_init()
	{
		m_primarySurfaceId = m_surfaceIdGenerator.generate();
		return true;
	}
	void surface_backend_service::service_clear()
	{
		m_primarySurfaceId = invalid_surface_id;
		m_surfaceIdGenerator.reset();
	}
}
