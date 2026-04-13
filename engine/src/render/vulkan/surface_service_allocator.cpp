#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
#include <engine/render/vulkan/surface_service_vulkan.h>
#else
#include <engine/render/surface_service.h>
#endif

namespace engine
{
	surface_service* surface_service::instance_allocate_vulkan()
	{
#if RATS_ENGINE_VULKAN_ENABLE
		return new surface_service_vulkan();
#else
		return nullptr;
#endif
	}
}
