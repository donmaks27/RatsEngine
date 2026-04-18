#include <engine/render/glfw/window_glfw_vulkan_service.h>

#include <engine/render/vulkan/render_vulkan_service.h>

#include <GLFW/glfw3.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window_glfw_vulkan_service)

    window_service* window_service::instance_allocate_vulkan() { return new window_glfw_vulkan_service(); }

    eastl::vector<const char*> window_glfw_vulkan_service::required_instance_extensions() const
    {
        uint32_t extensionCount = 0;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        return { extensions, extensions + extensionCount };
    }

    vk::SurfaceKHR window_glfw_vulkan_service::create_surface_impl(const vulkan::context& ctx, const window_id& id) const
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

    bool window_glfw_vulkan_service::on_event(const event_info& event)
    {
        return vulkan_super_t::handle_event(event);
    }

    bool window_glfw_vulkan_service::create_window_impl(const window_id& id, const window_create_info& info)
    {
        return super_t::create_window_impl(id, info) && vulkan_super_t::on_window_created(id);
    }
    void window_glfw_vulkan_service::destroy_window_impl(const window_id& id)
    {
        vulkan_super_t::on_window_destroying(id);
        super_t::destroy_window_impl(id);
    }
}
