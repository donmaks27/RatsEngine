#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/window_service.h>
#include <engine/render/vulkan/swapchain.h>

namespace engine
{
    class render_service_vulkan;

    class window_service_vulkan
    {
        friend render_service_vulkan;

    protected:
        window_service_vulkan() { Instance = this; }
        virtual ~window_service_vulkan() { Instance = nullptr; }
    public:
        window_service_vulkan(const window_service_vulkan&) = delete;
        window_service_vulkan(window_service_vulkan&&) = delete;

        window_service_vulkan& operator=(const window_service_vulkan&) = delete;
        window_service_vulkan& operator=(window_service_vulkan&&) = delete;

        [[nodiscard]] static constexpr auto logger() { return vulkan::logger_vulkan(window_service::logger()); }
        [[nodiscard]] static auto instance() { return Instance; }

        [[nodiscard]] vk::SurfaceKHR surface(const window_id& id) const;
		[[nodiscard]] vulkan::swapchain* swapchain(const window_id& id);

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const = 0;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface_impl(const vulkan::context& ctx, const window_id& id) const = 0;

        bool handle_event(const utils::event_info& event);

        [[nodiscard]] bool on_window_created(const window_id& id);
        void on_window_destroying(const window_id& id);

    private:

        static const log::logger Log;
        static window_service_vulkan* Instance;

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
