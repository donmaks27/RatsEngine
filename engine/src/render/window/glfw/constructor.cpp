module rats_engine.render;

import rats_engine.render.glfw;

namespace engine
{
	window_manager* window_manager::create_instance_impl(const create_info&)
	{
		return new window_manager_glfw();
	}
}