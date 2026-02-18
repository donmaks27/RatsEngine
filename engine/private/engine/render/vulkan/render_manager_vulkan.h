#pragma once

#include <engine/core.h>
#include <engine/render/render_manager.h>
#include <engine/render/window_manager.h>

#include <EASTL/vector_map.h>
#include <fmt/format.h>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class render_manager_vulkan;

    class vulkan_context final
    {
        friend render_manager_vulkan;

    public:
        vulkan_context() = default;
        vulkan_context(const vulkan_context&) = default;
        ~vulkan_context() = default;

        vulkan_context& operator=(const vulkan_context&) = default;

        [[nodiscard]] const vk::Instance& i() const { return m_instance; }
        [[nodiscard]] const vk::Device& d() const { return m_device; }

    private:

        vk::Instance m_instance;
        vk::Device m_device;
    };

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
        [[nodiscard]] virtual vk::SurfaceKHR create_surface(const vulkan_context& ctx, const window_id& id) const = 0;

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

        [[nodiscard]] bool on_instance_created(const vulkan_context& ctx);
    };

    class render_manager_vulkan final : public render_manager
    {
        using super = render_manager;

    public:
        render_manager_vulkan() = default;
        virtual ~render_manager_vulkan() override = default;

        [[nodiscard]] static render_manager_vulkan* instance() { return s_instanceVulkan; }

        [[nodiscard]] const vulkan_context& ctx() const { return m_ctx; }

    protected:

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

    private:

        static render_manager_vulkan* s_instanceVulkan;

        vulkan_context m_ctx;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::PhysicalDevice m_physicalDevice;

        [[nodiscard]] bool create_instance(const create_info& info);
        [[nodiscard]] bool create_device();
    };
}

template<>
struct fmt::formatter<vk::Result> : formatter<std::string>
{
    template<typename FormatContext>
    auto format(const vk::Result result, FormatContext& ctx) const
    {
        const auto value = fmt::format("{} (0x{:08X})", vk::to_string(result), static_cast<std::uint32_t>(result));
        return formatter<std::string>::format(value, ctx);
    }
};