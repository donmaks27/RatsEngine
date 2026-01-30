module;

#include <engine_export.h>

#include <cstdint>

#include <fmt/format.h>

export module rats_engine:render;

import :engine;
import :window;

export namespace engine
{
	enum class render_api : std::uint8_t { vulkan, opengl, directx11, directx12 };
	constexpr std::string_view render_api_to_string(const render_api api)
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

	class RATS_ENGINE_EXPORT render_manager
	{
		friend engine;

	public:
		render_manager() = default;
		virtual ~render_manager() = default;
	public:
		render_manager(const render_manager&) = delete;
		render_manager(render_manager&&) = delete;

		render_manager& operator=(const render_manager&) = delete;
		render_manager& operator=(render_manager&&) = delete;

	protected:

		window_manager* m_windowManager = nullptr;


		virtual bool init_render_manager_impl() { return false; }
		virtual void clear_render_manager_impl() {}

	private:

		[[nodiscard]] bool init();
		void clear();
	};
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
