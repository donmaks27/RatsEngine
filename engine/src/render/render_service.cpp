#include <engine/render/render_service.h>

#include <engine/render/window_service.h>

namespace engine
{
    render_service* render_service::Instance = nullptr;

#if !RATS_ENGINE_VULKAN_ENABLE
    render_service* render_service::instance_allocate_vulkan() { return nullptr; }
#endif

    bool render_service::service_init(const service_create_info& info)
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
