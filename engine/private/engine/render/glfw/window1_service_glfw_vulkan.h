#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>
#include <engine/render/glfw/window1_service_glfw.h>

namespace engine
{
    class window1_service_glfw_vulkan : public window1_service_glfw
    {
        RATS_ENGINE_SERVICE(window1_service_glfw_vulkan, "vulkan")

    protected:

        virtual bool service_init(const render_api_service_create_info&) override;
        virtual void service_clear() override;
    };
}
