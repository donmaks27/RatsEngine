#include <engine/render/glfw/window_manager_glfw_vulkan.h>

#include <GLFW/glfw3.h>

namespace engine
{
    eastl::vector<const char*> vulkan::window_manager_glfw::get_required_extensions() const
    {
        uint32_t extensionCount = 0;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return { extensions, extensions + extensionCount };
    }
}