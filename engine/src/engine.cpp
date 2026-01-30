module;

#include <engine/macro/defer.h>

module rats_engine;

import rats_engine.utils;

namespace engine
{
	engine::~engine()
	{
		if (is_started())
		{
			clear_engine();
		}
	}

	bool engine::start()
	{
		if (is_started())
		{
			log::error("[engine::start] Engine already started!");
			return false;
		}
		m_engineStarted = true;

		RATS_ENGINE_DEFER([this] { clear_engine(); });

		log::log("[engine::start] Initializing engine...");
		if (!init_engine())
		{
			log::fatal("[engine::start] Engine initialization failed!");
			return false;
		}
		log::info("[engine::start] Engine initialized successfully");

		log::log("[engine::start] Game loop started");
		auto* windowManager = m_renderManager->get_window_manager();
		while (!windowManager->shouldCloseMainWindow())
		{
			windowManager->on_frame_end();
		}
		log::log("[engine::start] Game loop stopped");
		return true;
	}

	bool engine::init_engine()
	{
		m_renderManager = render_manager::instance({ .api = render_api::vulkan });
		if (m_renderManager == nullptr)
		{
			return false;
		}
		return true;
	}

	void engine::clear_engine()
	{
		log::log("[engine::clear_engine] Clearing engine...");

		render_manager::clear_instance();
		m_renderManager = nullptr;

		log::info("[engine::clear_engine] Engine cleared successfully");
		m_engineStarted = false;
	}
}
