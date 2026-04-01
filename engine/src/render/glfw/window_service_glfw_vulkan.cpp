#include <engine/render/glfw/window_service_glfw_vulkan.h>

#include <engine/render/vulkan/render_service_vulkan.h>

#include <GLFW/glfw3.h>

namespace engine
{
    const log::logger window_service_glfw_vulkan::Log = window_service_glfw_vulkan::logger();

    eastl::vector<const char*> window_service_glfw_vulkan::required_instance_extensions() const
    {
        uint32_t extensionCount = 0;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return { extensions, extensions + extensionCount };
    }

    vk::SurfaceKHR window_service_glfw_vulkan::create_surface_impl(const vulkan::context& ctx, const window_id& id) const
    {
        const auto iter = m_windowDataGLFW.find(id);
        if (iter == m_windowDataGLFW.end())
        {
            return nullptr;
        }

        VkSurfaceKHR surface;
        const auto result = static_cast<vk::Result>(glfwCreateWindowSurface(*ctx.i(), iter->second, nullptr, &surface));
        if (result != vk::Result::eSuccess)
        {
            Log.error("Failed to create window surface: {}", result);
            return nullptr;
        }

        return surface;
    }

    bool window_service_glfw_vulkan::on_event(const event_info& event)
    {
        return super_vulkan::handle_event(event);
    }

    bool window_service_glfw_vulkan::create_window_impl(const window_id& id, const window_create_info& info)
    {
        return super::create_window_impl(id, info) && super_vulkan::on_window_created(id);
    }
    void window_service_glfw_vulkan::destroy_window_impl(const window_id& id)
    {
        super_vulkan::on_window_destroying(id);
        super::destroy_window_impl(id);
    }
}
