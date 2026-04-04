#pragma once

#include <engine/core.h>

#include <engine/events.h>

namespace engine
{
    class RATS_ENGINE_EXPORT engine_event_listener : public event_listener
    {
    protected:
        engine_event_listener();
        virtual ~engine_event_listener() override;
    };
}
