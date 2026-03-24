#pragma once

#include <engine/core.h>

#include <engine/utils/events.h>

namespace engine
{
    class RATS_ENGINE_EXPORT engine_event_listener : public utils::event_listener
    {
    protected:
        engine_event_listener();
        virtual ~engine_event_listener() override;
    };
}
