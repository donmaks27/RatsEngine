#pragma once

#include <engine/core.h>
#include <engine/service.h>

#include <engine/render/render_api.h>
#include <engine/render/surface_id.h>

#include <glm/vec2.hpp>

namespace engine
{
	class surface_backend_service;

	class RATS_ENGINE_EXPORT surface_service : public service
	{
		RATS_ENGINE_BASE_SERVICE(surface_service, "surface")

		friend surface_backend_service;

	public:

		struct surface_create_info
		{
			surface_create_info() = default;
			explicit surface_create_info(const glm::uvec2& size)
				: size(size)
			{}

			glm::uvec2 size = { 0, 0 };
		};

		[[nodiscard]] auto surface_ids() const
		{
			using pair_type = decltype(m_surfaces)::value_type;
			return m_surfaces | std::ranges::views::transform([](const pair_type& data) -> surface_id {
				return data.first;
			});
		}
		[[nodiscard]] bool surface_valid(const surface_id id) const { return m_surfaces.count(id) != 0; }
		[[nodiscard]] glm::uvec2 surface_size(const surface_id id) const { return m_surfaces.at_key(id).size; }

		void clear_surfaces();

	protected:

		virtual bool service_init() override;
		virtual void service_clear() override;

		[[nodiscard]] bool create_surface(surface_id id, const surface_create_info& info);
		virtual void destroy_surface(surface_id id);

	private:

		struct surface_data
		{
			glm::uvec2 size = { 0, 0 };
		};

		eastl::vector_map<surface_id, surface_data> m_surfaces;
	};

	class surface_backend_service : public service
	{
		RATS_ENGINE_BASE_SERVICE(surface_backend_service, "surface_backend")

	public:

		[[nodiscard]] surface_id primary_surface_id() const { return m_primarySurfaceId; }

		virtual void poll_events() = 0;

	protected:

		utils::id<surface_id> m_surfaceIdGenerator;

		virtual bool service_init() override;
		virtual void service_clear() override;

		static void destroy_surface(const surface_id id) { surface_service::instance().destroy_surface(id); }

	private:

		surface_id m_primarySurfaceId = invalid_surface_id;
	};

	[[nodiscard]] surface_service* allocate_surface_service_vulkan();
	[[nodiscard]] surface_backend_service* allocate_surface_backend_service_vulkan();

	[[nodiscard]] surface_service* allocate_surface_service();
	[[nodiscard]] surface_backend_service* allocate_surface_backend_service();
}
