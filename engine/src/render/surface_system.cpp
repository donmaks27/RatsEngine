#include <engine/render/surface_system.h>

namespace engine
{
	const log::logger surface_system::Log = surface_system::logger();

	surface_system* surface_system::s_instance = nullptr;
	surface_system* surface_system::create_instance(const create_info& info)
	{
		if (s_instance != nullptr)
		{
			return s_instance;
		}

		Log.log("Initializing surface system ({})...", info.api);
		s_instance = allocate_instance(info);
		if (s_instance == nullptr)
		{
			Log.fatal("Failed to allocate instance of surface system!");
			return nullptr;
		}

		Log.info("Surface system created successfully");
		return s_instance;
	}
	void surface_system::clear_instance()
	{
		if (s_instance != nullptr)
		{
			Log.log("Clearing surface system...");

			delete s_instance;
			s_instance = nullptr;
			Log.log("Surface system cleared successfully");
		}
	}
}
