#include <engine/render/render_service.h>

#include <engine/render/window_service.h>

namespace engine
{
    const log::logger render_service::Log = render_service::logger();
    render_service* render_service::Instance = nullptr;

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
