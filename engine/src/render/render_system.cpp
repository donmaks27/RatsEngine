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

        Log.log("Initializing render system ({})...", info.api);
        s_instance = allocate_instance(info);
        if (s_instance == nullptr)
        {
            Log.fatal("Failed to allocate instance of render system!");
            return nullptr;
        }
        if (!s_instance->init(info))
        {
            Log.fatal("Failed to initialize render system!");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            return nullptr;
        }
        Log.info("Render system initialized successfully");
        return s_instance;
    }
    void render_system::clear_instance()
    {
        if (s_instance != nullptr)
        {
            Log.log("Clearing render system...");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            Log.log("Render system cleared successfully");
        }
    }

    bool render_system::init(const create_info& info)
    {
        if (window_system::create_instance({ .api = info.api }) == nullptr)
        {
            Log.fatal("Failed to create window system!");
            return false;
        }
        return true;
    }

    void render_system::clear()
    {
        window_system::clear_instance();
    }
}
