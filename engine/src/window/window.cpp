module;

module rats_engine;

namespace engine
{
	bool window_manager::init()
	{
		log::log("[window_manager::init_window_manager] Initializing window manager...");
		if (!init_window_manager_impl())
		{
			log::fatal("[window_manager::init_window_manager] Failed to initialize window manager!");
			clear();
			return false;
		}
		log::info("[window_manager::init_window_manager] Window manager initialized successfully");
		return true;
	}

	void window_manager::clear()
	{
		log::log("[window_manager::clear_window_manager] Clearing window manager...");
		clear_window_manager_impl();
		log::info("[window_manager::clear_window_manager] Window manager cleared successfully");
	}
}
