#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
#include <engine/render/vulkan/surface_system_vulkan.h>
#else
#include <engine/render/surface_system.h>
#endif

namespace engine
{
	surface_system* surface_system::instance_allocate_vulkan()
	{
#if RATS_ENGINE_VULKAN_ENABLE
		return new surface_system_vulkan();
#else
		return nullptr;
#endif
	}
}
