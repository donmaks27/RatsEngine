#pragma once

#include <engine/core.h>

#include <engine/render/render_api.h>

namespace engine
{
	using surface_id = std::uint32_t;

	class RATS_ENGINE_EXPORT surface_system
	{
	protected:
		surface_system() = default;
		virtual ~surface_system() = default;
	public:
		surface_system(const surface_system&) = delete;
		surface_system(surface_system&&) = delete;

		surface_system& operator=(const surface_system&) = delete;
		surface_system& operator=(surface_system&&) = delete;

		[[nodiscard]] static constexpr log::logger logger() { return log::logger("surface_system", logger_engine()); }

		struct create_info
		{
			render_api api = render_api::vulkan;
		};
		static surface_system* create_instance(const create_info& info);
		[[nodiscard]] static surface_system* instance() { return s_instance; }
		static void clear_instance();

	private:

		static const log::logger Log;
		static surface_system* s_instance;
		static surface_system* allocate_instance(const create_info& info) { return nullptr; }


	};
}
