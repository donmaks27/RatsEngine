#pragma once

#include <engine/core.h>
#include <engine/render/glfw/window_glfw_service.h>
#include <engine/render/vulkan/window_vulkan_service.h>

namespace engine
{
    class window_glfw_vulkan_service : public window_glfw_service, public window_vulkan_service
    {
        RATS_ENGINE_SERVICE(window_glfw_vulkan_service, "vulkan")

        using vulkan_super_t = window_vulkan_service;

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const override;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface_impl(const vulkan::context& ctx, const window_id& id) const override;

        virtual bool on_event(const event_info& event) override;

        virtual bool create_window_impl(const window_id& id, const window_create_info& info) override;
        virtual void destroy_window_impl(const window_id& id) override;
    };
}