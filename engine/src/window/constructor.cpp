module rats_engine.window_contructor;

import rats_engine.utils;

namespace engine
{
	window_manager* create_window_manager()
	{
		log::error("[create_window_manager] Can't find any implementation of window_manager!");
		return nullptr;
	}
}
