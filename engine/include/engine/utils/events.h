// Copyright © 2026 Leonov Maksim. All rights reserved.

#pragma once

#include <engine/core.h>

#include <engine/utils/type_storage.h>

#include <cstdint>
#include <algorithm>

#include <EASTL/deque.h>
#include <EASTL/vector_map.h>
#include <EASTL/vector_set.h>
#include <EASTL/unique_ptr.h>

namespace engine::utils
{
    class event_bus;

    using event_id = std::uint32_t;
    inline constexpr event_id invalid_event_id = type_storage<event_id>::invalid_id;

    struct RATS_ENGINE_EXPORT event
    {
    protected:

        template<typename EventType>
        [[nodiscard]] static event_id event_type()
        {
            static const event_id id = TypeIds.get_type_id<EventType>();
            return id;
        }

    private:

        static type_storage<event_id> TypeIds;
    };
    template<typename EventType>
    struct event_of : event
    {
        [[nodiscard]] static event_id type() { return event_type<EventType>(); }
    };
    template<typename T>
    concept event_type = std::derived_from<T, event_of<T>> && std::is_final_v<T>;

    struct RATS_ENGINE_EXPORT event_info
    {
        const event& e;
        const event_id t;

        template<typename EventType, typename Func> requires event_type<EventType> && (std::invocable<Func, const EventType&> || std::invocable<Func>)
        bool dispatch(Func&& func) const
        {
            if (t == EventType::type())
            {
                if constexpr (std::predicate<Func, const EventType&>)
                {
                    return func(static_cast<const EventType&>(e));
                }
                else if constexpr (std::predicate<Func>)
                {
                    return func();
                }
                else if constexpr (std::invocable<Func, const EventType&>)
                {
                    func(static_cast<const EventType&>(e));
                }
                else
                {
                    func();
                }
            }
            return true;
        }
    };

    class RATS_ENGINE_EXPORT event_listener
    {
        friend event_bus;

    protected:
        event_listener() = default;
        virtual ~event_listener() = default;

        virtual bool on_event(const event_info& event) = 0;
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
        bool post_immediate(const EventType& event)
        {
            return std::ranges::all_of(m_listeners, [&event](event_listener* listener) {
                return listener->on_event(event_info{event, EventType::type()});
            });
        }

        template<typename EventType> requires event_type<EventType>
        void post(EventType&& event)
        {
            event_channel<EventType>().deferredEvents.push_back(std::forward<EventType>(event));
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
        eastl::vector_map<event_id, eastl::unique_ptr<channel_base>> m_eventChannels;

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
            m_eventChannels.emplace(id, eastl::unique_ptr<channel_base>(ch));
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
