#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>
#include <engine/render/surface_system.h>
#include <engine/engine_event_listener.h>

#include <engine/render/vulkan/swapchain.h>

namespace engine
{
	class surface_backend_system_vulkan;

	class surface_system_vulkan final : public surface_system, public engine_event_listener
	{
		using super = surface_system;

		friend surface_backend_system_vulkan;

	public:
		surface_system_vulkan() { Instance = this; }
		virtual ~surface_system_vulkan() override { Instance = nullptr; }

		[[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(super::logger()); }
		inline static const log::logger Log = logger();

		[[nodiscard]] static auto instance() { return Instance; }

		[[nodiscard]] vulkan::swapchain& surface_swapchain(const surface_id id) { return m_surfaces.at_key(id).swapchain; }
		[[nodiscard]] const vulkan::swapchain& surface_swapchain(const surface_id id) const { return m_surfaces.at_key(id).swapchain; }

	protected:

		virtual void service_clear() override;

		virtual bool on_event(const event_info& event) override;

	private:

		static surface_system_vulkan* Instance;

		struct vulkan_surface_create_info : surface_create_info
		{
			vk::SurfaceKHR surface = nullptr;
		};
		struct vulkan_surface_data
		{
			vk::SurfaceKHR surface = nullptr;
			vulkan::swapchain swapchain = nullptr;
		};

		eastl::vector_map<surface_id, vulkan_surface_data> m_surfaces;


		[[nodiscard]] surface_id create_surface(const vulkan::context& ctx, const vulkan_surface_create_info& info);
		void clear_surface(const vulkan::context& ctx, surface_id id);

		[[nodiscard]] bool create_swapchain(const vulkan::context& ctx, surface_id id);
		[[nodiscard]] bool on_device_created();
	};
}
