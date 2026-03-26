#pragma once

#include <engine/core.h>

#include <engine/render/render_api.h>

namespace engine
{
	template<typename CreateInfo>
	concept render_api_system_create_info = requires(CreateInfo info) {
		{ info.renderApi } -> std::convertible_to<render_api>;
	};

	template<typename SystemType, typename CreateInfo>
	class system
	{
		static constexpr bool RenderApiSystem = render_api_system_create_info<CreateInfo>;

	protected:
		system() = default;
		virtual ~system() = default;
	public:
		system(const system&) = delete;
		system(system&&) = delete;

		system& operator=(const system&) = delete;
		system& operator=(system&&) = delete;

		using instance_create_info = CreateInfo;

		[[nodiscard]] static SystemType* instance() { return SystemType::Instance; }
		[[nodiscard]] static bool instance_create(const instance_create_info& info)
		{
			if (SystemType::Instance == nullptr)
			{
				SystemType::Log.log("Creating system...");
				SystemType::Instance = system::instance_allocate(info);
				if (SystemType::Instance == nullptr)
				{
					SystemType::Log.fatal("Failed to allocate system!");
					return false;
				}
				if (!SystemType::Instance->system_init(info))
				{
					SystemType::Log.fatal("Failed to initialize system!");
					SystemType::Instance->system_clear();
					delete SystemType::Instance;
					SystemType::Instance = nullptr;
					return false;
				}
				SystemType::Log.info("System created successfully");
			}
			return true;
		}
		static void instance_clear()
		{
			if (SystemType::Instance != nullptr)
			{
				SystemType::Log.log("Clearing system...");
				SystemType::Instance->system_clear();
				delete SystemType::Instance;
				SystemType::Instance = nullptr;
				SystemType::Log.log("System cleared successfully");
			}
		}

	protected:

		virtual bool system_init(const instance_create_info& info) = 0;
		virtual void system_clear() = 0;

	private:

		static SystemType* instance_allocate(const instance_create_info& info)
		{
			if constexpr (!RenderApiSystem)
			{
				return SystemType::instance_allocate_impl(info);
			}
			else
			{
				SystemType* result = nullptr;
				switch (info.renderApi)
				{
				case render_api::vulkan: result = SystemType::instance_allocate_vulkan(); break;
				default:;
				}
				if (result == nullptr)
				{
					SystemType::Log.fatal("Render API '{}' is not implemented", info.renderApi);
				}
				return result;
			}
		}
	};

	struct default_render_api_system_create_info
	{
		render_api renderApi = render_api::vulkan;
	};
	template<typename SystemType, render_api_system_create_info CreateInfo = default_render_api_system_create_info>
	using render_api_system = system<SystemType, CreateInfo>;
}
