#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/window_manager.h>

namespace engine
{
    class render_manager_vulkan;

    class window_manager_vulkan
    {
        friend render_manager_vulkan;

    protected:
        window_manager_vulkan() = default;
        virtual ~window_manager_vulkan() = default;
    public:
        window_manager_vulkan(const window_manager_vulkan&) = delete;
        window_manager_vulkan(window_manager_vulkan&&) = delete;

        window_manager_vulkan& operator=(const window_manager_vulkan&) = delete;
        window_manager_vulkan& operator=(window_manager_vulkan&&) = delete;

        [[nodiscard]] static window_manager_vulkan* instance() { return s_instanceVulkan; }

        [[nodiscard]] vk::SurfaceKHR surface(const window_id& id) const;

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const = 0;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface(const window_id& id) const = 0;

        void on_init();
        void on_clear();

        void on_destroy_window(const window_id& id);

    private:

        struct window_data_vulkan
        {
            vk::SurfaceKHR surface;
        };

        static window_manager_vulkan* s_instanceVulkan;

        eastl::vector_map<window_id, window_data_vulkan> m_windowDataVulkan;

        void clear_vulkan();

        [[nodiscard]] bool on_instance_created();
    };
}
