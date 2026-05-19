#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>
#include <engine/render/surface_service.h>
#include <engine/engine_event_listener.h>

namespace engine
{
	class surface_vulkan_service final : public surface_service, public engine_event_listener
	{
		friend surface_service* allocate_surface_service_vulkan();

		RATS_ENGINE_SERVICE(surface_vulkan_service, render_api_to_string(render_api::vulkan))
	
	public:

		struct surface_vulkan_create_info : surface_create_info
		{
			surface_vulkan_create_info() = default;
			surface_vulkan_create_info(const glm::uvec2& size, const vk::SurfaceKHR& surface)
				: surface_create_info(size)
				, surface(surface)
			{}

			vk::SurfaceKHR surface = nullptr;
		};

		[[nodiscard]] static eastl::vector<const char*> required_instance_extensions();

		[[nodiscard]] vk::SurfaceKHR surface(const surface_id id) const { return m_surfaces.at_key(id).surface; }
		[[nodiscard]] vulkan::swapchain& surface_swapchain(const surface_id id) { return m_surfaces.at_key(id).swapchain; }
		[[nodiscard]] const vulkan::swapchain& surface_swapchain(const surface_id id) const { return m_surfaces.at_key(id).swapchain; }

		[[nodiscard]] bool create_surface(const vulkan::context& ctx, surface_id id, const surface_vulkan_create_info& info);
		[[nodiscard]] bool recreate_outdated_swapchains(const vulkan::context& ctx);

	protected:

		virtual void service_clear() override;

		virtual void on_event(const event_info& event) override;

		virtual void destroy_surface(surface_id id) override;

	private:

		struct vulkan_surface_data
		{
			vk::SurfaceKHR surface = nullptr;
			vulkan::swapchain swapchain = nullptr;
		};

		eastl::vector_map<surface_id, vulkan_surface_data> m_surfaces;


		[[nodiscard]] bool create_swapchain(const vulkan::context& ctx, surface_id id);
		void create_missing_swapchains();
	};
}
