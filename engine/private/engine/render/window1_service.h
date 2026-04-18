#pragma once

#include <engine/core.h>
#include <engine/render/surface_service.h>

namespace engine
{
    class window1_service : public surface_backend_service
    {
	    RATS_ENGINE_SERVICE(window1_service, "window")

    protected:

        struct window_data
        {
            bool minimized = false;
        };
        eastl::vector_map<surface_id, window_data> m_windowData;
        surface_id m_mainSurface = invalid_surface_id;

        virtual bool service_init(const render_api_service_create_info&) override;
        virtual void service_clear() override;
    };
}
