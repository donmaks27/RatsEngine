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
            Log.error("Engine already started!");
            return false;
        }
        m_engineStarted = true;

        RATS_ENGINE_DEFER([this] { clear_engine(); });

        Log.log("Initializing engine...");
        if (!init_engine())
        {
            Log.fatal("Engine initialization failed!");
            return false;
        }
        Log.info("Engine initialized successfully");

        Log.log("Game loop started");
        auto* windowSystem = window_system::instance();
        while (!windowSystem->should_close_main_window())
        {
            windowSystem->on_frame_end();

            m_engineEventBus.refresh_events();
        }
        Log.log("Game loop stopped");
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
        Log.log("Clearing engine...");

        render_system::clear_instance();

        Log.log("Engine cleared successfully");
        m_engineStarted = false;
    }
}