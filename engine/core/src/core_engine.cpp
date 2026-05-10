#include <engine/core_engine.h>

#include <engine/engine_event_listener.h>
#include <engine/render/render_service.h>
#include <engine/render/surface_service.h>
#include <engine/utils/macro/defer.h>

namespace engine
{
    utils::type_storage<event_id> event::TypeIds;
    utils::type_storage<service_type> service::ServiceTypes;

    core_engine* core_engine::Instance = nullptr;

    core_engine::core_engine(engine_config&& cfg)
    {
        Instance = this;
        m_engineConfig = std::move(cfg);
    }
    core_engine::~core_engine()
    {
        if (is_started())
        {
            Log.fatal("Engine was not properly cleared before destruction!");
        }
        Instance = nullptr;
    }

    bool core_engine::start()
    {
        if (is_started())
        {
            Log.error("Engine already started!");
            return false;
        }

        RATS_ENGINE_DEFER([this] { clear_engine(); });

        Log.log("Initializing engine...");
        if (!init_engine())
        {
            Log.fatal("Engine initialization failed!");
            return false;
        }
        Log.info("Engine initialized successfully");

        Log.log("Game loop started");
        auto& surfaceBackendService = surface_backend_service::instance();
        auto& renderService = render_service::instance();

        while (!m_signalShutdown)
        {
            m_engineEventBus.clear_events();

            if (!renderService.render())
            {
                Log.fatal("Render error!");
                break;
            }
            surfaceBackendService.poll_events();
        }
        Log.log("Game loop stopped");
        return true;
    }

    bool core_engine::init_engine()
    {
        m_engineStarted = true;

        return allocate_services()
            && m_serviceInstances[surface_service::type()]->service_init()
            && m_serviceInstances[surface_backend_service::type()]->service_init()
            && m_serviceInstances[render_service::type()]->service_init();
    }
    bool core_engine::allocate_services()
    {
        return register_service(allocate_surface_service)
            && register_service(allocate_surface_backend_service)
            && register_service(allocate_render_service);
    }

    void core_engine::clear_engine()
    {
        Log.log("Clearing engine...");

        m_serviceInstances[surface_service::type()]->service_clear();
        m_serviceInstances[surface_backend_service::type()]->service_clear();
        m_serviceInstances[render_service::type()]->service_clear();
        for (auto& service : m_serviceInstances)
        {
            delete service;
            service = nullptr;
        }

        Log.log("Engine cleared successfully");

        m_engineStarted = false;
    }

    engine_event_listener::engine_event_listener()
    {
        core_engine::instance().events().add_listener(this);
    }
    engine_event_listener::~engine_event_listener()
    {
        core_engine::instance().events().remove_listener(this);
    }
}
