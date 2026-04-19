#pragma once

#include <engine/core.h>
#include <engine/render/surface_service.h>

namespace engine
{
    class window1_service : public surface_backend_service
    {
	    RATS_ENGINE_SERVICE(window1_service, "window")

    public:

        struct window_create_info
        {
            glm::uvec2 size = { 0, 0 };
        };

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
