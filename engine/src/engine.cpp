#include <engine/engine.h>

#include <engine/render/render_system.h>
#include <engine/render/window_system.h>
#include <engine/utils/macro/defer.h>

namespace engine
{
    const log::logger engine::Log = engine::logger();

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
            Log.error("start: Engine already started!");
            return false;
        }
        m_engineStarted = true;

        RATS_ENGINE_DEFER([this] { clear_engine(); });

        Log.log("start: Initializing engine...");
        if (!init_engine())
        {
            Log.fatal("start: Engine initialization failed!");
            return false;
        }
        Log.info("start: Engine initialized successfully");

        Log.log("start: Game loop started");
        auto* windowManager = window_system::instance();
        while (!windowManager->should_close_main_window())
        {
            windowManager->on_frame_end();

            m_engineEventBus.refresh_events();
        }
        Log.log("start: Game loop stopped");
        return true;
    }

    bool engine::init_engine()
    {
        if (render_system::create_instance({ .api = render_api::vulkan }) == nullptr)
        {
            return false;
        }
        return true;
    }

    void engine::clear_engine()
    {
        Log.log("clear_engine: Clearing engine...");

        render_system::clear_instance();

        Log.log("clear_engine: Engine cleared successfully");
        m_engineStarted = false;
    }
}