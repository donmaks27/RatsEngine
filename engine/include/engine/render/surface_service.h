#pragma once

#include <engine/core.h>
#include <engine/service.h>

#include <glm/vec2.hpp>

#include <engine/render/vulkan/render_service_vulkan.h>

namespace engine
{
	using surface_id = std::uint8_t;
	constexpr surface_id invalid_surface_id = 0;

	class RATS_ENGINE_EXPORT surface_service : public service_of<surface_service, render_api_service_create_info>
	{
	public:
		using super = service_of;
        friend super;

	protected:
		surface_service() { Instance = this; }
		virtual ~surface_service() override { Instance = nullptr; }
	public:

		[[nodiscard]] static constexpr log::logger logger() { return { "surface", super::logger() }; }
		inline static const log::logger Log = logger();
		[[nodiscard]] static auto instance() { return Instance; }

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

		virtual bool service_init(const service_create_info& info) override { return false; }
		virtual void service_clear() override {}

		[[nodiscard]] surface_id create_surface(const surface_create_info& info);
		void clear_surface(surface_id id);

	private:

		static surface_service* Instance;
		[[nodiscard]] static surface_service* instance_allocate_vulkan();

		struct surface_data
		{
			glm::uvec2 size = { 0, 0 };
		};

		eastl::vector_map<surface_id, surface_data> m_surfaces;
	};

	class surface_backend_service : public service_of<surface_backend_service, render_api_service_create_info>
	{
	public:
		using super = service_of;
        friend super;

	protected:
		surface_backend_service() { Instance = this; }
		virtual ~surface_backend_service() override { Instance = nullptr; }
	public:

		inline static const log::logger Log = logger();
		[[nodiscard]] static constexpr log::logger logger() { return { "surface_backend", super::logger() }; }
		[[nodiscard]] static auto instance() { return Instance; }

		[[nodiscard]] virtual bool should_stop_engine() const { return false; }

	private:

		static surface_backend_service* Instance;
		[[nodiscard]] static surface_service* instance_allocate_vulkan() { return nullptr; }
	};
}
