#include <engine/render/render_system.h>

#include <engine/render/window_system.h>

namespace engine
{
    render_system* render_system::s_instance = nullptr;
    render_system* render_system::create_instance(const create_info& info)
    {
        if (s_instance != nullptr)
        {
            return s_instance;
        }

        log::log("[render_system::create_instance] Creating instance of render manager ({})...", info.api);
        s_instance = create_instance_impl(info);
        if (s_instance == nullptr)
        {
            log::fatal("[render_system::create_instance] Failed to create instance of render manager!");
            return nullptr;
        }
        if (!s_instance->init(info))
        {
            log::fatal("[render_system::create_instance] Failed to initialize render manager instance!");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            return nullptr;
        }
        log::info("[render_system::create_instance] Render manager instance created successfully");
        return s_instance;
    }
    void render_system::clear_instance()
    {
        if (s_instance != nullptr)
        {
            log::log("[render_system::clear_instance] Clearing instance of render manager...");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            log::log("[render_system::clear_instance] Render manager instance cleared successfully");
        }
    }

    bool render_system::init(const create_info& info)
    {
        if (window_system::create_instance({ .api = info.api }) == nullptr)
        {
            log::fatal("[render_system::init] Failed to get window manager instance!");
            return false;
        }
        return true;
    }

    void render_system::clear()
    {
        window_system::clear_instance();
    }
}