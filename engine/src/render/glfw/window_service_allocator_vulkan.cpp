#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
#include <engine/render/glfw/window_service_glfw_vulkan.h>
#else
#include <engine/render/window_service.h>
#endif

namespace engine
{
	window_service* window_service::instance_allocate_vulkan()
	{
#if RATS_ENGINE_VULKAN_ENABLE
		return new window_service_glfw_vulkan();
#else
		return nullptr;
#endif
	}
}
