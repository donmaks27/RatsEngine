#include <engine/utils/events.h>

namespace engine::utils
{
    event_id event_base::generate_event_type()
    {
        static event_id id = invalid_event_id;
        return ++id;
    }

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
