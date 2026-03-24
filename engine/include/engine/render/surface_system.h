#pragma once

#include <engine/core.h>
#include <engine/system.h>

#include <engine/render/render_api.h>

#include <glm/vec2.hpp>

namespace engine
{
	using surface_id = std::uint8_t;
	constexpr surface_id invalid_surface_id = 0;

	struct surface_system_create_info
	{
		render_api api = render_api::vulkan;
	};

	class RATS_ENGINE_EXPORT surface_system : public system<surface_system, surface_system_create_info>
	{
		friend system;

	protected:
		surface_system() = default;
		virtual ~surface_system() override = default;
	public:

		[[nodiscard]] static constexpr log::logger logger() { return log::logger("surface_system", logger_engine()); }

		[[nodiscard]] auto ids() const
		{
			using pair_type = decltype(m_surfaces)::value_type;
			return m_surfaces | std::ranges::views::transform([](const pair_type& data) -> surface_id {
				return data.first;
			});
		}
		[[nodiscard]] bool valid(const surface_id id) const { return m_surfaces.count(id) != 0; }
		[[nodiscard]] glm::uvec2 size(const surface_id id) const { return m_surfaces.at_key(id).size; }

	protected:

		virtual bool system_init(const instance_create_info& info) override { return false; }
		virtual void system_clear() override {}

		[[nodiscard]] surface_id on_surface_created(const glm::uvec2& size);
		void on_surface_cleared(surface_id id);

	private:

		static const log::logger Log;
		static surface_system* Instance;
		static surface_system* instance_allocate(const instance_create_info& info) { return nullptr; }

		struct surface_data
		{
			glm::uvec2 size = { 0, 0 };
		};

		eastl::vector_map<surface_id, surface_data> m_surfaces;
	};
}
