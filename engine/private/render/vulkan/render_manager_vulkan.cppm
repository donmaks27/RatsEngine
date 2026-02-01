module;

#include <string>

#include <EASTL/vector.h>
#include <fmt/format.h>

export module rats_engine.render.vulkan:render_manager;

import rats_engine.render;
import vulkan_hpp;

export namespace engine::vulkan
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

		[[nodiscard]] const vk::DispatchLoaderDynamic& l() const { return m_loader; }
		[[nodiscard]] const vk::Instance& i() const { return m_instance; }

	private:

		vk::DispatchLoaderDynamic m_loader;
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

export namespace vk
{
	constexpr std::string format_as(const Result& value)
	{
		return fmt::format("{} (0x{:08X})", to_string(value), static_cast<std::uint32_t>(value));
	}
}
