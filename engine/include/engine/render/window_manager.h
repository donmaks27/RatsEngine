#pragma once

#include <engine/core.h>

#include <engine/render/render_api.h>
#include <engine/utils/uuid.h>

#include <EASTL/vector_map.h>

#include <ranges>

namespace engine
{
    using window_id = utils::uuid;

    struct window_create_info {};

    class RATS_ENGINE_EXPORT window_manager
    {
    protected:
        window_manager() = default;
        virtual ~window_manager() = default;
    public:
        window_manager(const window_manager&) = delete;
        window_manager(window_manager&&) = delete;

        window_manager& operator=(const window_manager&) = delete;
        window_manager& operator=(window_manager&&) = delete;

        struct create_info
        {
            render_api api = render_api::vulkan;
        };
        static window_manager* create_instance(const create_info& info);
        [[nodiscard]] static window_manager* instance() { return s_instance; }
        static void clear_instance();

        [[nodiscard]] auto window_ids() const
        {
            using pair_type = decltype(m_windowData)::value_type;
            return m_windowData | std::ranges::views::transform([](const pair_type& data) -> const window_id& {
                return data.first;
            });
        }

        [[nodiscard]] window_id main_window_id() const { return m_mainWindowId; }
        [[nodiscard]] virtual bool should_close_window(const window_id& id) const = 0;
        [[nodiscard]] bool should_close_main_window() const { return should_close_window(main_window_id()); }

        virtual void on_frame_end() {}

    protected:

        struct window_data {};

        eastl::vector_map<window_id, window_data> m_windowData;
        window_id m_mainWindowId = window_id::invalid_id();

        [[nodiscard]] virtual bool init(const create_info& info);
        virtual void clear();

        [[nodiscard]] virtual bool create_window_impl(const window_id& id, const window_create_info& info) = 0;

    private:

        static window_manager* s_instance;
        static window_manager* create_instance_impl(const create_info& info);
    };
}