#pragma once

#include <engine/core.h>
#include <engine/service.h>
#include <engine/engine_event_listener.h>

#include <engine/utils/uuid.h>

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
    struct window_service_create_info
    {
        window_create_info mainWindow{};
        render_api renderApi = render_api::vulkan;
    };

    class RATS_ENGINE_EXPORT window_service : public service_of<window_service, window_service_create_info>, public engine_event_listener
    {
        RATS_ENGINE_SERVICE_BASE(window_service, "window")

    public:

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

        virtual bool service_init(const service_create_info_t& info) override;
        virtual void service_clear() override;

        [[nodiscard]] virtual bool create_window_impl(const window_id& id, const window_create_info& info) = 0;
        virtual void destroy_window_impl(const window_id& id) = 0;

    private:

        [[nodiscard]] static window_service* instance_allocate_vulkan();
    };
}