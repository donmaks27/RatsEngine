#include <engine/render/render_service.h>

#include <engine/render/surface_service.h>
#include <engine/render/window_service.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(render_service)

#if !RATS_ENGINE_VULKAN_ENABLE
    render_service* render_service::instance_allocate_vulkan() { return nullptr; }
#endif

    bool render_service::service_init(const service_create_info_t& info)
    {
        if (!surface_service::instance_create({ .renderApi = info.renderApi }))
        {
            Log.fatal("Failed to create surface service!");
            return false;
        }
        if (!surface_backend_service::instance_create({ .renderApi = info.renderApi }))
        {
            Log.fatal("Failed to create surface backend service!");
            surface_service::instance_clear();
            return false;
        }
        window_service::instance().create_window({ .size = { 1024, 768 } });
        return true;
    }

    void render_service::service_clear()
    {
        surface_backend_service::instance_clear();
        surface_service::instance_clear();
    }
}
