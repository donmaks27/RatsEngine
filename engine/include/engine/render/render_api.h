#pragma once

#include <engine/core.h>

#include <fmt/format.h>

namespace engine
{
    enum class render_api : std::uint8_t { vulkan, directx12 };

    [[nodiscard]] constexpr std::string_view render_api_to_string(const render_api api)
    {
        switch (api)
        {
        case render_api::vulkan: return "Vulkan";
        case render_api::directx12: return "DirectX12";
        default:;
        }
        return "NONE";
    }
}

template<>
struct fmt::formatter<engine::render_api> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const engine::render_api api, FormatContext& ctx) const
    {
        return formatter<std::string_view>::format(render_api_to_string(api), ctx);
    }
};