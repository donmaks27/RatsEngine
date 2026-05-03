#include <engine/render/window_service.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(window_service)

    bool window_service::service_init(const service_create_info_t& info)
    {
        if (!super_t::service_init(info))
        {
            return false;
        }
        if (!create_window_impl(primary_surface_id(), { .size = { 800, 600 } }))
        {
            Log.fatal("Failed to create main window!");
            return false;
        }
        return true;
    }
    void window_service::service_clear()
    {
        while (!m_windowData.empty())
        {
            destroy_window_impl(m_windowData.back().first);
        }
        super_t::service_clear();
    }

    surface_id window_service::create_window(const window_create_info& info)
    {
        const auto surfaceId = m_surfaceIdGenerator.generate();
        if (!create_window_impl(surfaceId, info))
        {
            m_surfaceIdGenerator.free(surfaceId);
            return invalid_surface_id;
        }
        return surfaceId;
    }
    void window_service::destroy_window(const surface_id id)
    {
        if (m_windowData.count(id) != 0)
        {
            destroy_window_impl(id);
            m_surfaceIdGenerator.free(id);
        }
    }

    bool window_service::create_window_impl(const surface_id id, const window_create_info& info)
    {
        m_windowData[id] = { .size = info.size };
        return true;
    }
    void window_service::destroy_window_impl(const surface_id id)
    {
        destroy_surface(id);
        m_windowData.erase(id);
    }
}
