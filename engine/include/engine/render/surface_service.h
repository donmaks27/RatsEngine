#pragma once

#include <engine/core.h>
#include <engine/service.h>

#include <engine/utils/id.h>

#include <glm/vec2.hpp>

namespace engine
{
	using surface_id = std::uint8_t;
	constexpr surface_id invalid_surface_id = utils::id<surface_id>::invalid_id;

	class surface_backend_service;

	class RATS_ENGINE_EXPORT surface_service : public service_of<surface_service, render_api_service_create_info>
	{
		RATS_ENGINE_SERVICE_BASE(surface_service, "surface")

		friend surface_backend_service;

	public:

		[[nodiscard]] auto surface_ids() const
		{
			using pair_type = decltype(m_surfaces)::value_type;
			return m_surfaces | std::ranges::views::transform([](const pair_type& data) -> surface_id {
				return data.first;
			});
		}
		[[nodiscard]] bool surface_valid(const surface_id id) const { return m_surfaces.count(id) != 0; }
		[[nodiscard]] glm::uvec2 surface_size(const surface_id id) const { return m_surfaces.at_key(id).size; }

	protected:

		struct surface_create_info
		{
			glm::uvec2 size = { 0, 0 };
		};

		virtual bool service_init(const service_create_info_t& info) override { return false; }
		virtual void service_clear() override {}

		[[nodiscard]] surface_id create_surface(const surface_create_info& info);
		virtual void clear_surface(surface_id id);

	private:

		[[nodiscard]] static surface_service* instance_allocate_vulkan();

		struct surface_data
		{
			glm::uvec2 size = { 0, 0 };
		};

		eastl::vector_map<surface_id, surface_data> m_surfaces;
	};

	class surface_backend_service : public service_of<surface_backend_service, render_api_service_create_info>
	{
		RATS_ENGINE_SERVICE_BASE(surface_backend_service, "surface_backend")

	protected:

		utils::id<surface_id> m_surfaceIdGenerator;

		virtual void service_clear() override;

	private:

		[[nodiscard]] static surface_backend_service* instance_allocate_vulkan();
	};
}
