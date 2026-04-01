#include <engine/events.h>

namespace engine
{
    utils::type_storage<event_id> event::TypeIds;

    void event_bus::add_listener(event_listener* listener)
    {
        m_listeners.insert(listener);
    }
    void event_bus::remove_listener(event_listener* listener)
    {
        m_listeners.erase(listener);
    }

    void event_bus::refresh_events()
    {
        for (auto& [id, ch] : m_eventChannels)
        {
            ch->refresh_events();
        }
    }
}
