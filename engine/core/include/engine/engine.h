#pragma once

#include <engine/core.h>

#include <engine/events.h>

namespace engine
{
    class RATS_ENGINE_EXPORT engine final
    {
        engine() = default;
        ~engine();
    public:
        engine(const engine&) = delete;
        engine(engine&&) = delete;

        engine& operator=(const engine&) = delete;
        engine& operator=(engine&&) = delete;

        [[nodiscard]] static constexpr log::logger logger() { return logger_engine(); }

        static engine& instance()
        {
            static engine engineInstance;
            return engineInstance;
        }

        bool start();
        [[nodiscard]] bool is_started() const { return m_engineStarted; }

        [[nodiscard]] event_bus& events() { return m_engineEventBus; }

        void signal_shutdown() { m_signalShutdown = true; }

    private:

        static const log::logger Log;

        event_bus m_engineEventBus{};

        bool m_engineStarted = false;
        bool m_signalShutdown = false;


        bool init_engine();
        void clear_engine();
    };
}