#pragma once

#include <engine/core.h>
#include <engine/render/glfw/window_manager_glfw.h>
#include <engine/render/vulkan/render_manager_vulkan.h>

namespace engine::vulkan
{
    class window_manager_glfw : public engine::window_manager_glfw, public engine::vulkan::window_manager
    {
        using super = engine::window_manager_glfw;

    public:
        window_manager_glfw() = default;
        virtual ~window_manager_glfw() override = default;

        [[nodiscard]] virtual eastl::vector<const char*> get_required_extensions() const override;
    };
}