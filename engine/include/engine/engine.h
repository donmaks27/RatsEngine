#pragma once

#include <engine/core.h>

#include <engine/utils/events.h>

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

        static engine& instance()
        {
            static engine engineInstance;
            return engineInstance;
        }

        bool start();
        [[nodiscard]] bool is_started() const { return m_engineStarted; }

        [[nodiscard]] utils::event_bus& event_bus() { return m_engineEventBus; }

    private:

        utils::event_bus m_engineEventBus{};

        bool m_engineStarted = false;


        bool init_engine();
        void clear_engine();
    };
}