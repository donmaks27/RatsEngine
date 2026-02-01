module;

#include <string>

#include <EASTL/vector.h>
#include <fmt/format.h>

export module rats_engine.render.vulkan:render_manager;

import rats_engine.render;
import vulkan_hpp;

export namespace engine
{
	class window_manager_vulkan
	{
	protected:
		window_manager_vulkan() = default;
		virtual ~window_manager_vulkan() = default;
	public:
		window_manager_vulkan(const window_manager_vulkan&) = delete;
		window_manager_vulkan(window_manager_vulkan&&) = delete;

		window_manager_vulkan& operator=(const window_manager_vulkan&) = delete;
		window_manager_vulkan& operator=(window_manager_vulkan&&) = delete;

		[[nodiscard]] virtual eastl::vector<const char*> get_required_extensions() const = 0;
	};

	class render_manager_vulkan final : public render_manager
	{
		using super = render_manager;

	public:
		render_manager_vulkan() = default;
		virtual ~render_manager_vulkan() override = default;

	protected:

		virtual bool init(const create_info& info) override;
		virtual void clear() override;

	private:

		window_manager_vulkan* m_windowManagerVulkan = nullptr;

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
