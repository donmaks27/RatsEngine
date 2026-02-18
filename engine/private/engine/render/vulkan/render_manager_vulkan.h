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

    class vulkan_render_object
    {
        friend render_manager_vulkan;

    protected:
        vulkan_render_object() = default;
        ~vulkan_render_object() = default;

        [[nodiscard]] inline const vulkan_context& api_ctx() const;

    private:

        render_manager_vulkan* m_renderManagerVulkan = nullptr;
    };

    class window_manager_vulkan : public vulkan_render_object
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

        [[nodiscard]] vk::SurfaceKHR surface(const window_id& id) const;

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const = 0;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface(const vulkan_context& ctx, const window_id& id) const = 0;

        void on_destroy_window(const window_id& id);

    private:

        struct window_data_vulkan
        {
            vk::SurfaceKHR surface;
        };

        eastl::vector_map<window_id, window_data_vulkan> m_windowDataVulkan;

        void clear_vulkan();

        [[nodiscard]] bool on_instance_created(const vulkan_context& ctx);
    };

    class render_manager_vulkan final : public engine::render_manager
    {
        using super = engine::render_manager;

    public:
        render_manager_vulkan() = default;
        virtual ~render_manager_vulkan() override = default;

        [[nodiscard]] const vulkan_context& api_ctx() const { return m_apiCtx; }

    protected:

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

    private:

        window_manager_vulkan* m_windowManagerVulkan = nullptr;

        vulkan_context m_apiCtx;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::PhysicalDevice m_physicalDevice;

        [[nodiscard]] bool create_instance(const create_info& info);
        [[nodiscard]] bool create_device();
    };

    const vulkan_context& vulkan_render_object::api_ctx() const
    {
        return m_renderManagerVulkan->api_ctx();
    }
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