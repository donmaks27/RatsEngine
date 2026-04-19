#pragma once

#include <engine/core.h>
#include <engine/render/surface_service.h>

namespace engine
{
    class window_service : public surface_backend_service
    {
	    RATS_ENGINE_SERVICE(window_service, "window")

    public:

        struct window_create_info
        {
            glm::uvec2 size = { 0, 0 };
        };

        [[nodiscard]] auto window_ids() const
        {
            using pair_type = decltype(m_windowData)::value_type;
            return m_windowData | std::ranges::views::transform([](const pair_type& data) -> surface_id {
                return data.first;
            });
        }
        [[nodiscard]] surface_id main_window_id() const { return m_mainSurfaceID; }
        [[nodiscard]] virtual bool should_close_main_window() const = 0;

        [[nodiscard]] glm::uvec2 window_size(const surface_id id) const { return m_windowData.at_key(id).size; }

        virtual void poll_window_events() = 0;

        void destroy_window(surface_id id);

    protected:

        virtual bool service_init(const render_api_service_create_info&) override;
        virtual void service_clear() override;

        [[nodiscard]] virtual bool create_window_impl(surface_id id, const window_create_info& info);
        virtual void destroy_window_impl(surface_id id);

    private:

        struct window_data
        {
            glm::uvec2 size = { 0, 0 };
        };

        eastl::vector_map<surface_id, window_data> m_windowData;
        surface_id m_mainSurfaceID = invalid_surface_id;
    };
}
