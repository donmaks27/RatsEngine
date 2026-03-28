#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
#include <engine/render/vulkan/render_service_vulkan.h>
#else
#include <engine/render/render_system.h>
#endif

namespace engine
{
	render_service* render_service::instance_allocate_vulkan()
	{
#if RATS_ENGINE_VULKAN_ENABLE
		return new render_service_vulkan();
#else
		return nullptr;
#endif
	}
}
