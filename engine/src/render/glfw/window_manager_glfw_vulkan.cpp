#include <engine/render/glfw/window_manager_glfw_vulkan.h>

#include <GLFW/glfw3.h>

namespace engine
{
    eastl::vector<const char*> vulkan::window_manager_glfw::required_instance_extensions() const
    {
        uint32_t extensionCount = 0;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return { extensions, extensions + extensionCount };
    }

    vk::SurfaceKHR vulkan::window_manager_glfw::create_surface(const api_context& ctx, const window_id& id) const
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
            log::error("[vulkan::window_manager_glfw::create_surface] Failed to create window surface: {}", result);
            return nullptr;
        }

        return surface;
    }

    void vulkan::window_manager_glfw::destroy_window_impl(const window_id& id)
    {
        vulkan::window_manager::on_destroy_window(id);
        super::destroy_window_impl(id);
    }
}
