module;

#include <string>

#include <EASTL/vector.h>
#include <fmt/format.h>

export module rats_engine.render.vulkan:render_manager;

import rats_engine.render;
import vulkan_hpp;

export namespace engine::vulkan
{
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

	class render_manager final : public engine::render_manager
	{
		using super = engine::render_manager;

	public:
		render_manager() = default;
		virtual ~render_manager() override = default;

	protected:

		virtual bool init(const create_info& info) override;
		virtual void clear() override;

	private:

		window_manager* m_windowManagerVulkan = nullptr;

		vk::Instance m_instance;
		vk::DispatchLoaderDynamic m_loader;
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
