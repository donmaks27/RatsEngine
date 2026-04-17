#include <engine/render/render_service.h>

#include <engine/render/window_service.h>

namespace engine
{
    RATS_ENGINE_SERVICE_IMPL(render_service)

#if !RATS_ENGINE_VULKAN_ENABLE
    render_service* render_service::instance_allocate_vulkan() { return nullptr; }
#endif

    bool render_service::service_init(const service_create_info_t& info)
    {
        if (!window_service::instance_create({ .renderApi = info.renderApi }))
        {
            Log.fatal("Failed to create window system!");
            return false;
        }
        return true;
    }

    void render_service::service_clear()
    {
        window_service::instance_clear();
    }
}
