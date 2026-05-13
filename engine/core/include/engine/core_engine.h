#pragma once

#include <engine/core.h>

#include <engine/events.h>
#include <engine/service.h>
#include <engine/render/render_api.h>

#include <EASTL/array.h>

namespace engine
{
    struct engine_config
    {
        std::string appName = "RatsEngine";
        render_api renderApi = render_api::vulkan;
    };

    class RATS_ENGINE_EXPORT core_engine
    {
    protected:
        explicit core_engine(engine_config&& cfg);
        virtual ~core_engine();
    public:
        core_engine() = delete;
        core_engine(const core_engine&) = delete;
        core_engine(core_engine&&) = delete;

        core_engine& operator=(const core_engine&) = delete;
        core_engine& operator=(core_engine&&) = delete;

        [[nodiscard]] static auto& instance() { return *Instance; }

        bool start();
        [[nodiscard]] bool is_started() const { return m_engineStarted; }

        [[nodiscard]] const engine_config& config() const { return m_engineConfig; }
        [[nodiscard]] event_bus& events() { return m_engineEventBus; }

        void signal_shutdown() { m_signalShutdown = true; }

    protected:

        virtual void register_services(registration::service_storage& storage);

    private:

        static core_engine* Instance;

        const log::logger Log = logger_engine();

        engine_config m_engineConfig;
        event_bus m_engineEventBus;

        eastl::array<service*, std::numeric_limits<service_type>::max()> m_serviceInstances{};
        eastl::vector<service_type> m_serviceInitOrder;

        bool m_engineStarted = false;
        bool m_signalShutdown = false;


        [[nodiscard]] bool init_engine();
        [[nodiscard]] bool init_services(registration::service_storage& storage);

        void clear_engine();
    };
}
