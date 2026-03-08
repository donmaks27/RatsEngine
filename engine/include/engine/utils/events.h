// Copyright © 2026 Leonov Maksim. All rights reserved.

#pragma once

#include <engine/core.h>

#include <memory>

#include <EASTL/deque.h>
#include <EASTL/vector_map.h>
#include <EASTL/vector_set.h>

namespace engine::utils
{
    class event_bus;

    using event_id = std::uint32_t;
    inline constexpr event_id invalid_event_id = 0;

    struct RATS_ENGINE_EXPORT event_base
    {
    protected:
        event_base() = default;

        [[nodiscard]] static event_id generate_event_type();
    };
    template<typename EventType>
    struct event : private event_base
    {
        [[nodiscard]] static event_id type()
        {
            static event_id id = generate_event_type();
            return id;
        }
    };
    template<typename T>
    concept event_type = std::derived_from<T, event<T>> && std::is_final_v<T>;

    class RATS_ENGINE_EXPORT event_listener
    {
        friend event_bus;

    protected:
        event_listener() = default;
        virtual ~event_listener() = default;

        virtual void on_event(const event_base& event, event_id eventType) = 0;
    };

    class RATS_ENGINE_EXPORT event_bus final
    {
    public:
        event_bus() = default;
        event_bus(const event_bus&) = delete;
        event_bus(event_bus&&) noexcept = default;
        ~event_bus() = default;

        event_bus& operator=(const event_bus&) = delete;
        event_bus& operator=(event_bus&&) noexcept = default;

        template<typename EventType> requires event_type<EventType>
        struct events_list
        {
            friend event_bus;

            events_list() = delete;
        private:
            explicit events_list(const eastl::deque<EventType>* e) : events(e) {}
        public:

            [[nodiscard]] auto begin() const { return events != nullptr ? events->cbegin() : dummy.cbegin(); }
            [[nodiscard]] auto end() const { return events != nullptr ? events->cend() : dummy.cend(); }

        private:

            const eastl::deque<EventType>* events = nullptr;
            const eastl::deque<EventType> dummy;
        };

        void add_listener(event_listener* listener);
        void remove_listener(event_listener* listener);

        template<typename EventType> requires event_type<EventType>
        void post(EventType&& event)
        {
            event_channel<EventType>().deferredEvents.push_back(std::forward<EventType>(event));
        }
        template<typename EventType> requires event_type<EventType>
        void post_immediate(const EventType& event)
        {
            for (const auto& listener : m_listeners)
            {
                listener->on_event(event, EventType::type());
            }
        }

        void refresh_events();
        template<typename EventType> requires event_type<EventType>
        events_list<EventType> events() const
        {
            const auto* ch = event_channel_ptr<EventType>();
            return events_list<EventType>(ch != nullptr ? &ch->events : nullptr);
        }

    private:

        struct channel_base
        {
            virtual ~channel_base() = default;

            void refresh_events() { on_refresh_events(this); }

        protected:

            using callback_t = void(*)(channel_base*);
            callback_t on_refresh_events = nullptr;
        };
        template<typename EventType> requires event_type<EventType>
        struct channel : channel_base
        {
            channel()
            {
                on_refresh_events = [](channel_base* ch) {
                    reinterpret_cast<channel*>(ch)->refresh_events_impl();
                };
            }

            eastl::deque<EventType> events;
            eastl::deque<EventType> deferredEvents;

            void refresh_events_impl()
            {
                events.clear();
                std::swap(events, deferredEvents);
            }
        };

        eastl::vector_set<event_listener*> m_listeners;
        eastl::vector_map<event_id, std::unique_ptr<channel_base>> m_eventChannels;

        template<typename EventType> requires event_type<EventType>
        channel<EventType>& event_channel()
        {
            static event_id id = EventType::type();
            const auto iter = m_eventChannels.find(id);
            if (iter != m_eventChannels.end())
            {
                return *reinterpret_cast<channel<EventType>*>(iter->second.get());
            }
            auto* ch = new channel<EventType>();
            m_eventChannels.emplace(id, std::unique_ptr<channel_base>(ch));
            return *ch;
        }
        template<typename EventType> requires event_type<EventType>
        channel<EventType>* event_channel_ptr() const
        {
            static event_id id = EventType::type();
            const auto iter = m_eventChannels.find(id);
            return iter != m_eventChannels.end() ? reinterpret_cast<channel<EventType>*>(iter->second.get()) : nullptr;
        }
    };
}
