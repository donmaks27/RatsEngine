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

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const override;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface(const api_context& ctx, const window_id& id) const override;

        virtual void destroy_window_impl(const window_id& id) override;
    };
}