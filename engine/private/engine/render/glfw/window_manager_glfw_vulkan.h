#pragma once

#include <engine/core.h>
#include <engine/render/glfw/window_manager_glfw.h>
#include <engine/render/vulkan/render_manager_vulkan.h>

namespace engine
{
    class window_manager_glfw_vulkan : public window_manager_glfw, public window_manager_vulkan
    {
        using super = window_manager_glfw;
        using super_vulkan = window_manager_vulkan;

    public:
        window_manager_glfw_vulkan() = default;
        virtual ~window_manager_glfw_vulkan() override = default;

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const override;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface(const vulkan_context& ctx, const window_id& id) const override;

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

        virtual void destroy_window_impl(const window_id& id) override;
    };
}