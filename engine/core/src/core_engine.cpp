#include <engine/core_engine.h>

#include <engine/engine_event_listener.h>
#include <engine/render/render_service.h>
#include <engine/render/surface_service.h>
#include <engine/utils/macro/defer.h>

namespace engine
{
    utils::type_storage<event_id> event::TypeIds;

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

        registration::service_storage storage;
        register_services(storage);
        return init_services(storage);
    }
    void core_engine::register_services(registration::service_storage& storage)
    {
        storage.register_service<surface_service>()
            .allocator(allocate_surface_service)
            .init_before<surface_backend_service>();
        storage.register_service<surface_backend_service>()
            .allocator(allocate_surface_backend_service)
            .init_before<render_service>();
        storage.register_service<render_service>()
            .allocator(allocate_render_service);
    }
    bool core_engine::init_services(registration::service_storage& storage)
    {
        eastl::vector_map<service_type, eastl::vector_set<service_type>> dependencies;
        using dep_value_t = decltype(dependencies)::value_type;
        for (const auto& [type, service] : storage.m_services)
        {
            auto& currentDependencies = dependencies[type];
            for (const auto initAfterType : service.m_initAfter)
            {
                currentDependencies.insert(initAfterType);
            }
            for (const auto initBeforeType : service.m_initBefore)
            {
                dependencies[initBeforeType].insert(type);
            }
        }

        eastl::vector<service_type> tempServiceList;
        while (!dependencies.empty())
        {
            auto serviceWithoutDeps = dependencies | std::views::filter([](const dep_value_t& pair) {
                return pair.second.empty();
            }) | std::views::transform([](const dep_value_t& pair) {
                return pair.first;
            });
            if (serviceWithoutDeps.empty())
            {
                Log.fatal("Service dependencies loop detected!");
                return false;
            }
            std::ranges::copy(serviceWithoutDeps, std::back_inserter(tempServiceList));

            for (const auto& type : tempServiceList)
            {
                dependencies.erase(type);
                std::ranges::for_each(dependencies, [type](dep_value_t& pair) {
                    pair.second.erase(type);
                });
                m_serviceInitOrder.push_back(type);
            }
            tempServiceList.clear();
        }

        for (const auto& [type, info] : storage.m_services)
        {
            const auto& allocator = info.m_allocator;
            service* instance = allocator != nullptr ? allocator() : nullptr;
            if (instance == nullptr)
            {
                Log.fatal("Failed to allocate some service!");
                return false;
            }
            m_serviceInstances[type] = instance;
        }
        const bool initSuccess = std::ranges::all_of(m_serviceInitOrder, [this](const service_type type) {
            return m_serviceInstances[type]->service_init();
        });
        if (!initSuccess)
        {
            Log.fatal("Failed to initialize some service!");
            return false;
        }
        return true;
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
        m_serviceInitOrder.clear();

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
