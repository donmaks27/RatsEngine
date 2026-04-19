#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>
#include <engine/render/glfw/window_glfw_service.h>
#include <engine/engine_event_listener.h>

namespace engine
{
    class window_glfw_vulkan_service : public window_glfw_service, public engine_event_listener
    {
        RATS_ENGINE_SERVICE(window_glfw_vulkan_service, "vulkan")

    protected:

        virtual bool service_init(const render_api_service_create_info&) override;
        virtual void service_clear() override;

        virtual bool on_event(const event_info& event) override;

        virtual bool create_window_impl(surface_id id, const window_create_info& info) override;

    private:

        bool on_instance_created() const;
        [[nodiscard]] bool create_surface(const vulkan::context& ctx, surface_id id) const;
    };
}
