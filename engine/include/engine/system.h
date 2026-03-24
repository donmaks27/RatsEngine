#pragma once

#include <engine/core.h>

namespace engine
{
	template<typename SystemType, typename CreateInfo>
	class system
	{
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
				SystemType::Instance = SystemType::instance_allocate(info);
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
	};
}
