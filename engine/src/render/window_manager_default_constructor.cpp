module rats_engine.render;

import rats_engine.utils;

namespace engine
{
	window_manager* window_manager::create_instance_impl(const create_info&)
	{
		log::error("[window_manager::create_instance_impl] Can't find any implementation of window manager!");
		return nullptr;
	}
}
