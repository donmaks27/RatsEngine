#include <engine/render/render_system.h>

#include <engine/render/window_system.h>

namespace engine
{
    const log::logger render_system::Log = render_system::logger();
    render_system* render_system::Instance = nullptr;

    bool render_system::system_init(const instance_create_info& info)
    {
        if (!window_system::instance_create({ .renderApi = info.renderApi }))
        {
            Log.fatal("Failed to create window system!");
            return false;
        }
        return true;
    }

    void render_system::system_clear()
    {
        window_system::instance_clear();
    }
}
