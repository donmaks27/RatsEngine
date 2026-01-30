module;

module rats_engine.render;

import rats_engine.utils;

namespace engine
{
	render_manager* render_manager::s_instance = nullptr;
	render_manager* render_manager::instance(const create_info& info)
	{
		if (s_instance != nullptr)
		{
			return s_instance;
		}

		log::log("[render_manager::instance] Creating instance of render manager...");
		s_instance = create_instance_impl(info);
		if (s_instance == nullptr)
		{
			log::fatal("[render_manager::instance] Failed to create instance of render manager!");
			return nullptr;
		}
		if (!s_instance->init())
		{
			log::fatal("[render_manager::instance] Failed to initialize render manager instance!");
			s_instance->clear();
			delete s_instance;
			s_instance = nullptr;
			return nullptr;
		}
		log::info("[render_manager::instance] Render manager instance created successfully");
		return s_instance;
	}
	void render_manager::clear_instance()
	{
		if (s_instance != nullptr)
		{
			log::log("[render_manager::clear_instance] Clearing instance of render manager...");
			s_instance->clear();
			delete s_instance;
			s_instance = nullptr;
			log::info("[render_manager::clear_instance] Render manager instance cleared successfully");
		}
	}
}
