module;

#include <engine/macro/defer.h>

module rats_engine;

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
			log::error("[engine::start]: Engine already started!");
			return false;
		}
		m_engineStarted = true;

		RATS_ENGINE_DEFER([this] { clear_engine(); });
		log::log("[engine::start]: Initializing engine...");
		if (!init_engine())
		{
			log::fatal("[engine::start]: Engine initialization failed!");
			return false;
		}
		log::info("[engine::start]: Engine initialized successfully");

		log::log("[engine::start]: Game loop started");
		// Game loop would go here
		log::log("[engine::start]: Game loop stopped");
		return true;
	}

	bool engine::init_engine()
	{
		return true;
	}

	void engine::clear_engine()
	{
		log::log("[engine::clear_engine]: Clearing engine...");

		log::info("[engine::clear_engine]: Engine cleared successfully");
		m_engineStarted = false;
	}
}
