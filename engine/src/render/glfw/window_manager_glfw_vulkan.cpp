module;

#include <EASTL/vector.h>
#include <GLFW/glfw3.h>

module rats_engine.render.glfw.vulkan;

import rats_engine.render;

namespace engine
{
    window_manager* window_manager::create_instance_impl_vulkan(const create_info&)
    {
        return new window_manager_glfw_vulkan();
    }

    eastl::vector<const char*> window_manager_glfw_vulkan::get_required_extensions() const
    {
        uint32_t extensionCount = 0;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return { extensions, extensions + extensionCount };
    }
}
