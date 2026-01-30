module;

module rats_engine;

namespace engine
{
	bool render_manager::init()
	{
		log::log("[render_manager::init_render_manager] Initializing render manager...");
		if (!init_render_manager_impl())
		{
			log::fatal("[render_manager::init_render_manager] Failed to initialize render manager!");
			clear();
			return false;
		}
		log::info("[render_manager::init_render_manager] Render manager initialized successfully");
		return true;
	}

	void render_manager::clear()
	{
		log::log("[render_manager::clear_render_manager] Clearing render manager...");
		clear_render_manager_impl();
		log::info("[render_manager::clear_render_manager] Render manager cleared successfully");
	}
}
