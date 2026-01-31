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

	RATS_ENGINE_EXPORT constexpr std::string_view format_as(const render_api api)
	{
		return render_api_to_string(api);
	}
}
