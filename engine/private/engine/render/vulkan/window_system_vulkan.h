#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/window_system.h>
#include <engine/render/vulkan/swapchain.h>

namespace engine
{
    class render_system_vulkan;

    class window_system_vulkan
    {
        friend render_system_vulkan;

    protected:
        window_system_vulkan() = default;
        virtual ~window_system_vulkan() = default;
    public:
        window_system_vulkan(const window_system_vulkan&) = delete;
        window_system_vulkan(window_system_vulkan&&) = delete;

        window_system_vulkan& operator=(const window_system_vulkan&) = delete;
        window_system_vulkan& operator=(window_system_vulkan&&) = delete;

        [[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(window_system::logger()); }

        [[nodiscard]] static window_system_vulkan* instance() { return s_instanceVulkan; }

        [[nodiscard]] vk::SurfaceKHR surface(const window_id& id) const;
		[[nodiscard]] vulkan::swapchain* swapchain(const window_id& id);

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const = 0;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface_impl(const vulkan::context& ctx, const window_id& id) const = 0;

        bool handle_event(const utils::event_info& event);

        void on_init();
        static void on_clear();

        [[nodiscard]] bool on_window_created(const window_id& id);
        void on_window_destroying(const window_id& id);

    private:

        static const log::logger Log;
        static window_system_vulkan* s_instanceVulkan;

        struct window_data_vulkan
        {
            vk::SurfaceKHR surface = nullptr;
			vulkan::swapchain swapchain{};
        };

        eastl::vector_map<window_id, window_data_vulkan> m_windowDataVulkan;


        [[nodiscard]] bool on_instance_created();
        [[nodiscard]] bool on_device_created();

        [[nodiscard]] bool create_surface(const vulkan::context& ctx, const window_id& id);
		[[nodiscard]] bool create_swapchain(const vulkan::context& ctx, const window_id& id);

        void clear_vulkan(const vulkan::context& ctx);
    };
}
