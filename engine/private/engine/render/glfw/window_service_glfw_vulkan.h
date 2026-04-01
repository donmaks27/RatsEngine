#pragma once

#include <engine/core.h>
#include <engine/render/glfw/window_service_glfw.h>
#include <engine/render/vulkan/window_service_vulkan.h>

namespace engine
{
    class window_service_glfw_vulkan : public window_service_glfw, public window_service_vulkan
    {
        using super = window_service_glfw;
        using super_vulkan = window_service_vulkan;

    public:
        window_service_glfw_vulkan() = default;
        virtual ~window_service_glfw_vulkan() override = default;

        [[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(super::logger()); }

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const override;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface_impl(const vulkan::context& ctx, const window_id& id) const override;

        virtual bool on_event(const event_info& event) override;

        virtual bool create_window_impl(const window_id& id, const window_create_info& info) override;
        virtual void destroy_window_impl(const window_id& id) override;

    private:

        static const log::logger Log;
    };
}