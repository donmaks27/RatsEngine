#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
#include <engine/render/glfw/window_system_glfw_vulkan.h>
#else
#include <engine/render/window_system.h>
#endif

namespace engine
{
	window_system* window_system::instance_allocate_vulkan()
	{
#if RATS_ENGINE_VULKAN_ENABLE
		return new window_system_glfw_vulkan();
#else
		return nullptr;
#endif
	}
}
