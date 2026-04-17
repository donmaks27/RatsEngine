#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>
#include <engine/render/surface_service.h>
#include <engine/engine_event_listener.h>

#include <engine/render/vulkan/swapchain.h>

namespace engine
{
	class surface_backend_system_vulkan;

	struct vulkan_surface_created_event : event_of<vulkan_surface_created_event>
	{
		vk::SurfaceKHR surface = nullptr;
		glm::uvec2 size = { 0, 0 };
		surface_id id = invalid_surface_id;
	};

	class surface_service_vulkan final : public surface_service, public engine_event_listener
	{
		RATS_ENGINE_SERVICE(surface_service_vulkan, "vulkan")

	public:
		friend surface_backend_system_vulkan;

		[[nodiscard]] vulkan::swapchain& surface_swapchain(const surface_id id) { return m_surfaces.at_key(id).swapchain; }
		[[nodiscard]] const vulkan::swapchain& surface_swapchain(const surface_id id) const { return m_surfaces.at_key(id).swapchain; }

	protected:

		virtual void service_clear() override;

		virtual bool on_event(const event_info& event) override;

	private:

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
