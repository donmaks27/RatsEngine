export module rats_engine.render_constructor;

import rats_engine;

namespace engine
{
	render_manager* create_render_manager_vulkan() { return new render_manager(); }

	export inline render_manager* create_render_manager(const render_api api)
	{
		switch (api)
		{
		case render_api::vulkan: return create_render_manager_vulkan();
		default:;
		}
		log::error("[create_render_manager] Render API ({}) is not supported", api);
		return nullptr;
	}
}
