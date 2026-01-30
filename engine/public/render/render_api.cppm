module;

#include <engine_export.h>

#include <fmt/format.h>

export module rats_engine.render:render_api;

export namespace engine
{
	enum class render_api : std::uint8_t { vulkan, opengl, directx11, directx12 };
	[[nodiscard]] RATS_ENGINE_EXPORT constexpr std::string_view render_api_to_string(const render_api api)
	{
		switch (api)
		{
		case render_api::vulkan: return "Vulkan";
		case render_api::opengl: return "OpenGL";
		case render_api::directx11: return "DirectX11";
		case render_api::directx12: return "DirectX12";
		default:;
		}
		return "NONE";
	}
}

export template<>
struct RATS_ENGINE_EXPORT fmt::formatter<engine::render_api> : formatter<std::string_view>
{
	template <typename FormatContext>
	auto format(const engine::render_api api, FormatContext& ctx) const
	{
		return formatter<std::string_view>::format(engine::render_api_to_string(api), ctx);
	}
};