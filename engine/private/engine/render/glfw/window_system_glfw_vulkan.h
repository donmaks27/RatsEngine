#pragma once

#include <engine/core.h>
#include <engine/render/glfw/window_system_glfw.h>
#include <engine/render/vulkan/window_system_vulkan.h>

namespace engine
{
    class window_system_glfw_vulkan : public window_system_glfw, public window_system_vulkan
    {
        using super = window_system_glfw;
        using super_vulkan = window_system_vulkan;

    public:
        window_system_glfw_vulkan() = default;
        virtual ~window_system_glfw_vulkan() override = default;

        [[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(super::logger()); }

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const override;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface_impl(const vulkan::context& ctx, const window_id& id) const override;

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

        virtual bool on_event(const utils::event_info& event) override;

        virtual bool create_window_impl(const window_id& id, const window_create_info& info) override;
        virtual void destroy_window_impl(const window_id& id) override;

    private:

        static const log::logger Log;
    };
}