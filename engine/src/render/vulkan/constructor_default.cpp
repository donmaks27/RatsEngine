module rats_engine.render;

import rats_engine.utils;

namespace engine
{
	render_manager* render_manager::create_instance_impl_vulkan(const create_info& info)
	{
		log::error("[render_manager::create_instance_impl_vulkan] Render API '{}' is not supported", render_api::vulkan);
		return nullptr;
	}
}
