module;

#include <engine/macro/defer.h>

module rats_engine;

import rats_engine.window_contructor;

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
		while (!m_windowManager->shouldCloseMainWindow())
		{
			m_windowManager->on_frame_end();
		}
		log::log("[engine::start] Game loop stopped");
		return true;
	}

	bool engine::init_engine()
	{
		log::log("[engine::init_engine] Creating window manager...");
		m_windowManager = create_window_manager();
		if (m_windowManager == nullptr)
		{
			log::fatal("[engine::init_engine] Failed to create window manager!");
			return false;
		}
		if (!m_windowManager->init_window_manager())
		{
			log::fatal("[engine::init_engine] Window manager initialization failed!");
			return false;
		}
		log::info("[engine::init_engine] Window manager created successfully");

		return true;
	}

	void engine::clear_engine()
	{
		log::log("[engine::clear_engine] Clearing engine...");

		if (m_windowManager != nullptr)
		{
			log::log("[engine::clear_engine] Clearing window manager...");
			m_windowManager->clear_window_manager();
			delete m_windowManager;
			m_windowManager = nullptr;
			log::info("[engine::clear_engine] Window manager cleared successfully");
		}

		log::info("[engine::clear_engine] Engine cleared successfully");
		m_engineStarted = false;
	}
}
