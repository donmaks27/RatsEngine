#include <typeindex>
#include <engine/engine.h>

#include <engine/engine_event_listener.h>
#include <engine/render/render_service.h>
#include <engine/render/window_service.h>
#include <engine/utils/macro/defer.h>

namespace engine
{
    utils::type_storage<event_id> event::TypeIds;

    utils::type_storage<service_type> service::ServiceTypes;
    eastl::array<service*, std::numeric_limits<service_type>::max()> service::ServiceInstances;

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
        auto* windowSystem = window_service::instance();
        auto* renderSystem = render_service::instance();
        while (!windowSystem->should_close_main_window())
        {
            if (!renderSystem->render())
            {
                Log.fatal("Render error!");
                break;
            }

            windowSystem->on_frame_end();

            m_engineEventBus.refresh_events();
        }
        Log.log("Game loop stopped");
        return true;
    }

    bool engine::init_engine()
    {
        if (!render_service::instance_create({ .renderApi = render_api::vulkan }))
        {
            return false;
        }
        return true;
    }

    void engine::clear_engine()
    {
        Log.log("Clearing engine...");

        render_service::instance_clear();

        Log.log("Engine cleared successfully");
        m_engineStarted = false;
    }

    engine_event_listener::engine_event_listener()
    {
        engine::instance().events().add_listener(this);
    }
    engine_event_listener::~engine_event_listener()
    {
        engine::instance().events().remove_listener(this);
    }
}
