#pragma once

#include <engine/core.h>
#include <engine/render/render_manager.h>

#include <EASTL/vector.h>
#include <fmt/format.h>
#include <vulkan/vulkan.hpp>

namespace engine::vulkan
{
    class render_manager;

    class window_manager
    {
    protected:
        window_manager() = default;
        virtual ~window_manager() = default;
    public:
        window_manager(const window_manager&) = delete;
        window_manager(window_manager&&) = delete;

        window_manager& operator=(const window_manager&) = delete;
        window_manager& operator=(window_manager&&) = delete;

        [[nodiscard]] virtual eastl::vector<const char*> get_required_extensions() const = 0;
    };

    class render_api_context final
    {
        friend render_manager;

    public:
        render_api_context() = default;
        render_api_context(const render_api_context&) = default;
        ~render_api_context() = default;

        render_api_context& operator=(const render_api_context&) = default;

        [[nodiscard]] const vk::Instance& i() const { return m_instance; }

    private:

        vk::Instance m_instance;
    };

    class render_manager final : public engine::render_manager
    {
        using super = engine::render_manager;

    public:
        render_manager() = default;
        virtual ~render_manager() override = default;

        [[nodiscard]] const render_api_context& api_ctx() const { return m_apiCtx; }

    protected:

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

    private:

        window_manager* m_windowManagerVulkan = nullptr;

        render_api_context m_apiCtx;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
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