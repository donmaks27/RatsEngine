#include <engine/render/glfw/window_manager_glfw_vulkan.h>

#include <GLFW/glfw3.h>

namespace engine
{
    eastl::vector<const char*> window_manager_glfw_vulkan::required_instance_extensions() const
    {
        uint32_t extensionCount = 0;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return { extensions, extensions + extensionCount };
    }

    vk::SurfaceKHR window_manager_glfw_vulkan::create_surface(const vulkan_context& ctx, const window_id& id) const
    {
        const auto iter = m_windowDataGLFW.find(id);
        if (iter == m_windowDataGLFW.end())
        {
            return nullptr;
        }

        VkSurfaceKHR surface;
        const auto result = static_cast<vk::Result>(glfwCreateWindowSurface(ctx.i(), iter->second, nullptr, &surface));
        if (result != vk::Result::eSuccess)
        {
            log::error("[window_manager_glfw_vulkan::create_surface] Failed to create window surface: {}", result);
            return nullptr;
        }

        return surface;
    }

    void window_manager_glfw_vulkan::destroy_window_impl(const window_id& id)
    {
        window_manager_vulkan::on_destroy_window(id);
        super::destroy_window_impl(id);
    }
}
