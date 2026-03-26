#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>
#include <engine/render/surface_system.h>
#include <engine/engine_event_listener.h>

#include <engine/render/vulkan/swapchain.h>

namespace engine
{
	class surface_system_vulkan final : public surface_system, public engine_event_listener
	{
		using super = surface_system;

	public:
		surface_system_vulkan() = default;
		virtual ~surface_system_vulkan() override = default;

		[[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(super::logger()); }
		[[nodiscard]] static auto instance() { return Instance; }

		[[nodiscard]] surface_id create_surface(const vulkan::context& ctx, vk::SurfaceKHR surface, const glm::uvec2& size);
		void clear_surface(const vulkan::context& ctx, surface_id id);

		[[nodiscard]] vulkan::swapchain& surface_swapchain(const surface_id id) { return m_surfaces.at_key(id).swapchain; }
		[[nodiscard]] const vulkan::swapchain& surface_swapchain(const surface_id id) const { return m_surfaces.at_key(id).swapchain; }

	protected:

		virtual bool system_init(const instance_create_info& info) override;
		virtual void system_clear() override;

		virtual bool on_event(const utils::event_info& event) override;

	private:

		static const log::logger Log;
		static surface_system_vulkan* Instance;

		struct surface_data_vulkan
		{
			vk::SurfaceKHR surface = nullptr;
			vulkan::swapchain swapchain = nullptr;
		};

		eastl::vector_map<surface_id, surface_data_vulkan> m_surfaces;


		[[nodiscard]] bool create_swapchain(const vulkan::context& ctx, surface_id id);
		[[nodiscard]] bool on_device_created();
	};
}
