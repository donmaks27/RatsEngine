module;

#include <engine/macro/defer.h>

module rats_engine;

import rats_engine.window_constructor;
import rats_engine.render_constructor;

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
		{
			log::log("[engine::init_engine] Creating window manager...");
			auto windowManager = create_window_manager();
			if (windowManager == nullptr)
			{
				log::fatal("[engine::init_engine] Failed to create window manager!");
				return false;
			}
			log::info("[engine::init_engine] Window manager created successfully");
			if (!windowManager->init())
			{
				delete windowManager;
				return false;
			}
			m_windowManager = windowManager;
		}

		{
			log::log("[engine::init_engine] Creating render manager ({})...", render_api::vulkan);
			auto renderManager = create_render_manager(render_api::vulkan);
			if (renderManager == nullptr)
			{
				log::fatal("[engine::init_engine] Failed to create render manager!");
				return false;
			}
			log::info("[engine::init_engine] render manager created successfully");
			if (!renderManager->init())
			{
				delete renderManager;
				return false;
			}
			m_renderManager = renderManager;
		}
		return true;
	}

	void engine::clear_engine()
	{
		log::log("[engine::clear_engine] Clearing engine...");

		if (m_renderManager != nullptr)
		{
			m_renderManager->clear();
			delete m_renderManager;
			m_renderManager = nullptr;
		}
		if (m_windowManager != nullptr)
		{
			m_windowManager->clear();
			delete m_windowManager;
			m_windowManager = nullptr;
		}

		log::info("[engine::clear_engine] Engine cleared successfully");
		m_engineStarted = false;
	}
}
