module rats_engine.window_constructor;

import rats_engine.glfw;

namespace engine
{
	window_manager* create_window_manager()
	{
		return new window_manager_glfw();
	}
}