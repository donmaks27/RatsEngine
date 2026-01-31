module rats_engine.render.vulkan;

import rats_engine.render;
import rats_engine.utils;

namespace engine
{
	render_manager* render_manager::create_instance_impl_vulkan(const create_info& info)
	{
		return new render_manager_vulkan();
	}
}