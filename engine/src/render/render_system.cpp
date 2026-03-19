#include <engine/render/render_system.h>

#include <engine/engine.h>
#include <engine/render/window_system.h>

namespace engine
{
    const log::logger render_system::Log = render_system::logger();

    render_system* render_system::s_instance = nullptr;
    render_system* render_system::create_instance(const create_info& info)
    {
        if (s_instance != nullptr)
        {
            return s_instance;
        }

        Log.log("Creating instance of render system ({})...", info.api);
        s_instance = create_instance_impl(info);
        if (s_instance == nullptr)
        {
            Log.fatal("Failed to create instance of render system!");
            return nullptr;
        }
        if (!s_instance->init(info))
        {
            Log.fatal("Failed to initialize render system instance!");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            return nullptr;
        }
        Log.info("Render system instance created successfully");
        return s_instance;
    }
    void render_system::clear_instance()
    {
        if (s_instance != nullptr)
        {
            Log.log("Clearing instance of render system...");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            Log.log("Render system instance cleared successfully");
        }
    }

    bool render_system::init(const create_info& info)
    {
        if (window_system::create_instance({ .api = info.api }) == nullptr)
        {
            Log.fatal("Failed to get window system instance!");
            return false;
        }
        return true;
    }

    void render_system::clear()
    {
        window_system::clear_instance();
    }
}
