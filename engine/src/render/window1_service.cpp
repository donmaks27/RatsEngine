#include <engine/render/window1_service.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window1_service)

    bool window1_service::service_init(const service_create_info_t& info)
    {
        // Create main window
        m_mainSurfaceID = m_surfaceIdGenerator.generate();
        if (!create_window_impl(m_mainSurfaceID, { .size = { 800, 600 } }))
        {
            Log.fatal("Failed to create main window!");
            m_surfaceIdGenerator.free(m_mainSurfaceID);
            m_mainSurfaceID = invalid_surface_id;
            return false;
        }
        return true;
    }
    void window1_service::service_clear()
    {
        while (!m_windowData.empty())
        {
            destroy_window_impl(m_windowData.back().first);
        }
        m_mainSurfaceID = invalid_surface_id;
        super_t::service_clear();
    }

    void window1_service::destroy_window(const surface_id id)
    {
        if (m_windowData.count(id) != 0)
        {
            destroy_window_impl(id);
            m_surfaceIdGenerator.free(id);
        }
    }

    bool window1_service::create_window_impl(const surface_id id, const window_create_info& info)
    {
        m_windowData[id] = { .size = info.size };
        return true;
    }
    void window1_service::destroy_window_impl(const surface_id id)
    {
        destroy_surface(id);
        m_windowData.erase(id);
    }
}
