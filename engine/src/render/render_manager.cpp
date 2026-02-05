#include <engine/render/render_manager.h>

#include <engine/render/window_manager.h>

namespace engine
{
    render_manager* render_manager::s_instance = nullptr;
    render_manager* render_manager::create_instance(const create_info& info)
    {
        if (s_instance != nullptr)
        {
            return s_instance;
        }

        log::log("[render_manager::create_instance] Creating create_instance of render manager ({})...", info.api);
        s_instance = create_instance_impl(info);
        if (s_instance == nullptr)
        {
            log::fatal("[render_manager::create_instance] Failed to create create_instance of render manager!");
            return nullptr;
        }
        if (!s_instance->init(info))
        {
            log::fatal("[render_manager::create_instance] Failed to initialize render manager create_instance!");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            return nullptr;
        }
        log::info("[render_manager::create_instance] Render manager create_instance created successfully");
        return s_instance;
    }
    void render_manager::clear_instance()
    {
        if (s_instance != nullptr)
        {
            log::log("[render_manager::clear_instance] Clearing instance of render manager...");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            log::info("[render_manager::clear_instance] Render manager instance cleared successfully");
        }
    }

    render_manager* render_manager::create_instance_impl(const create_info& info)
    {
        switch (info.api)
        {
        case render_api::vulkan: return create_instance_impl_vulkan(info);
        default:;
        }
        log::fatal("[render_manager::create_instance_impl] Render API '{}' is not implemented", info.api);
        return nullptr;
    }

    bool render_manager::init(const create_info& info)
    {
        if (window_manager::create_instance({ .api = info.api }) == nullptr)
        {
            log::fatal("[render_manager::init] Failed to get window manager instance!");
            return false;
        }
        return true;
    }

    void render_manager::clear()
    {
        window_manager::clear_instance();
    }
}