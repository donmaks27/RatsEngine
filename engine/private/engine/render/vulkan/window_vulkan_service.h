#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/window_service.h>
#include <engine/render/vulkan/swapchain.h>

namespace engine
{
    class render_vulkan_service;

    class window_vulkan_service
    {
        friend render_vulkan_service;

    protected:
        window_vulkan_service() { Instance = this; }
        virtual ~window_vulkan_service() { Instance = nullptr; }
    public:
        window_vulkan_service(const window_vulkan_service&) = delete;
        window_vulkan_service(window_vulkan_service&&) = delete;

        window_vulkan_service& operator=(const window_vulkan_service&) = delete;
        window_vulkan_service& operator=(window_vulkan_service&&) = delete;

        [[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(window_service::logger()); }
        inline static const log::logger Log = logger();

        [[nodiscard]] static auto instance() { return Instance; }

        [[nodiscard]] vk::SurfaceKHR surface(const window_id& id) const;
		[[nodiscard]] vulkan::swapchain* swapchain(const window_id& id);

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const = 0;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface_impl(const vulkan::context& ctx, const window_id& id) const = 0;

        bool handle_event(const event_info& event);

        [[nodiscard]] bool on_window_created(const window_id& id);
        void on_window_destroying(const window_id& id);

    private:

        static window_vulkan_service* Instance;

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
