#pragma once

#include <engine/core.h>
#include <engine/system.h>

#include <engine/render/render_api.h>

namespace engine
{
	using surface_id = std::uint32_t;

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
		surface_system(const surface_system&) = delete;
		surface_system(surface_system&&) = delete;

		surface_system& operator=(const surface_system&) = delete;
		surface_system& operator=(surface_system&&) = delete;

		[[nodiscard]] static constexpr log::logger logger() { return log::logger("surface_system", logger_engine()); }

	protected:

		virtual bool system_init(const instance_create_info& info) override { return false; }
		virtual void system_clear() override {}

	private:

		static const log::logger Log;
		static surface_system* Instance;
		static surface_system* instance_allocate(const instance_create_info& info) { return nullptr; }
	};
}
