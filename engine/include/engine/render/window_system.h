#pragma once

#include <engine/core.h>
#include <engine/system.h>

#include <engine/render/render_api.h>
#include <engine/utils/uuid.h>
#include <engine/utils/events.h>

#include <EASTL/vector_map.h>
#include <glm/vec2.hpp>

#include <ranges>

namespace engine
{
    using window_id = utils::uuid;

    struct window_create_info
    {
		glm::uvec2 size = { 800, 600 };
    };
    struct window_system_create_info
    {
        render_api renderApi = render_api::vulkan;
        window_create_info mainWindow{};
    };

    class RATS_ENGINE_EXPORT window_system : public utils::event_listener, public render_api_system<window_system, window_system_create_info>
    {
        friend system;

    protected:
        window_system();
        virtual ~window_system() override;
    public:

        [[nodiscard]] static constexpr log::logger logger() { return log::logger("window_system", logger_engine()); }

        [[nodiscard]] auto window_ids() const
        {
            using pair_type = decltype(m_windowData)::value_type;
            return m_windowData | std::ranges::views::transform([](const pair_type& data) -> const window_id& {
                return data.first;
            });
        }
		[[nodiscard]] glm::uvec2 window_size(const window_id& id) const;

        window_id create_window(const window_create_info& info);
        [[nodiscard]] virtual bool should_close_window(const window_id& id) const = 0;
        bool destroy_window(const window_id& id);

        [[nodiscard]] window_id main_window_id() const { return m_mainWindowId; }
        [[nodiscard]] bool should_close_main_window() const { return should_close_window(main_window_id()); }

        virtual void on_frame_end() {}

    protected:

        struct window_data
        {
			glm::uvec2 size = { 0, 0 };
        };

        eastl::vector_map<window_id, window_data> m_windowData;
        window_id m_mainWindowId = window_id::invalid_id();

        [[nodiscard]] virtual bool system_init(const instance_create_info& info) override;
        virtual void system_clear() override;

        [[nodiscard]] virtual bool create_window_impl(const window_id& id, const window_create_info& info) = 0;
        virtual void destroy_window_impl(const window_id& id) = 0;

    private:

        static const log::logger Log;
        static window_system* Instance;
        static window_system* instance_allocate_vulkan();
    };
}