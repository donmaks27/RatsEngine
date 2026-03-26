#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
#include <engine/render/vulkan/render_system_vulkan.h>
#else
#include <engine/render/render_system.h>
#endif

namespace engine
{
	render_system* render_system::instance_allocate_vulkan()
	{
#if RATS_ENGINE_VULKAN_ENABLE
		return new render_system_vulkan();
#else
		return nullptr;
#endif
	}
}
