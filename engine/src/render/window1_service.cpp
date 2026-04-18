#include <engine/render/window1_service.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window1_service)

    bool window1_service::service_init(const service_create_info_t& info)
    {
        // Create main window
        return true;
    }
    void window1_service::service_clear()
    {
        m_windowData.clear();
        m_mainSurface = invalid_surface_id;
        super_t::service_clear();
    }
}
