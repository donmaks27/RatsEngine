module rats_engine.window.contructor;

import rats_engine.window.glfw;

namespace engine
{
	window_manager* create_window_manager()
	{
		return new window_manager_glfw();
	}
}