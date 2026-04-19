#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>
#include <engine/render/glfw/window1_glfw_service.h>

namespace engine
{
    class window1_glfw_vulkan_service : public window1_glfw_service
    {
        RATS_ENGINE_SERVICE(window1_glfw_vulkan_service, "vulkan")

    protected:

        virtual bool service_init(const render_api_service_create_info&) override;
        virtual void service_clear() override;

        virtual bool create_window_impl(surface_id id, const window_create_info& info) override;
    };
}
