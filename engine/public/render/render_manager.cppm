module;

#include <engine_export.h>

#include <cstdint>

#include <fmt/format.h>

export module rats_engine.render:render_manager;

import :window_manager;

export namespace engine
{
	enum class render_api : std::uint8_t { vulkan, opengl, directx11, directx12 };
	[[nodiscard]] constexpr std::string_view render_api_to_string(const render_api api)
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
	protected:
		render_manager() = default;
		virtual ~render_manager() = default;
	public:
		render_manager(const render_manager&) = delete;
		render_manager(render_manager&&) = delete;

		render_manager& operator=(const render_manager&) = delete;
		render_manager& operator=(render_manager&&) = delete;

		struct create_info
		{
			render_api api = render_api::vulkan;
		};
		static render_manager* instance(const create_info& info);
		static void clear_instance();

	protected:

		window_manager* m_windowManager = nullptr;


		[[nodiscard]] virtual bool init() { return false; }
		virtual void clear() {}

	private:

		static render_manager* s_instance;

		static render_manager* create_instance_impl(const create_info& info) { return nullptr; }
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
