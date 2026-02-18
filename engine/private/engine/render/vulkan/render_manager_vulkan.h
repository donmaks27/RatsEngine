#pragma once

#include <engine/core.h>
#include <engine/render/render_manager.h>
#include <engine/render/window_manager.h>

#include <EASTL/vector_map.h>
#include <fmt/format.h>
#include <vulkan/vulkan.hpp>

namespace engine::vulkan
{
    class render_manager;

    class api_context final
    {
        friend render_manager;

    public:
        api_context() = default;
        api_context(const api_context&) = default;
        ~api_context() = default;

        api_context& operator=(const api_context&) = default;

        [[nodiscard]] const vk::Instance& i() const { return m_instance; }
        [[nodiscard]] const vk::Device& d() const { return m_device; }

    private:

        vk::Instance m_instance;
        vk::Device m_device;
    };

    class render_object
    {
        friend render_manager;

    protected:
        render_object() = default;
        ~render_object() = default;

        [[nodiscard]] inline const api_context& api_ctx() const;

    private:

        render_manager* m_renderManagerVulkan = nullptr;
    };

    class window_manager : public render_object
    {
        friend render_manager;

    protected:
        window_manager() = default;
        virtual ~window_manager() = default;
    public:
        window_manager(const window_manager&) = delete;
        window_manager(window_manager&&) = delete;

        window_manager& operator=(const window_manager&) = delete;
        window_manager& operator=(window_manager&&) = delete;

        [[nodiscard]] vk::SurfaceKHR surface(const window_id& id) const;

    protected:

        [[nodiscard]] virtual eastl::vector<const char*> required_instance_extensions() const = 0;
        [[nodiscard]] virtual vk::SurfaceKHR create_surface(const api_context& ctx, const window_id& id) const = 0;

        void on_destroy_window(const window_id& id);

    private:

        struct window_data_vulkan
        {
            vk::SurfaceKHR surface;
        };

        eastl::vector_map<window_id, window_data_vulkan> m_windowDataVulkan;

        void clear_vulkan();

        [[nodiscard]] bool on_instance_created(const api_context& ctx);
    };

    class render_manager final : public engine::render_manager
    {
        using super = engine::render_manager;

    public:
        render_manager() = default;
        virtual ~render_manager() override = default;

        [[nodiscard]] const api_context& api_ctx() const { return m_apiCtx; }

    protected:

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

    private:

        window_manager* m_windowManagerVulkan = nullptr;

        api_context m_apiCtx;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::PhysicalDevice m_physicalDevice;

        [[nodiscard]] bool create_instance(const create_info& info);
        [[nodiscard]] bool create_device();
    };

    const api_context& render_object::api_ctx() const
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