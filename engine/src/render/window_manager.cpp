#include <engine/render/window_manager.h>

namespace engine
{
    window_manager* window_manager::s_instance = nullptr;
    window_manager* window_manager::create_instance(const create_info& info)
    {
        if (s_instance != nullptr)
        {
            return s_instance;
        }

        log::log("[window_manager::create_instance] Creating create_instance of window manager ({})...", info.api);
        s_instance = create_instance_impl(info);
        if (s_instance == nullptr)
        {
            log::fatal("[window_manager::create_instance] Failed to create create_instance of window manager!");
            return nullptr;
        }
        if (!s_instance->init(info))
        {
            log::fatal("[window_manager::create_instance] Failed to initialize window manager create_instance!");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            return nullptr;
        }
        log::info("[window_manager::create_instance] Window manager create_instance created successfully");
        return s_instance;
    }
    void window_manager::clear_instance()
    {
        if (s_instance != nullptr)
        {
            log::log("[window_manager::clear_instance] Clearing instance of window manager...");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            log::info("[window_manager::clear_instance] Window manager instance cleared successfully");
        }
    }

    window_manager* window_manager::create_instance_impl(const create_info& info)
    {
        switch (info.api)
        {
        case render_api::vulkan: return create_instance_impl_vulkan(info);
        default:;
        }
        log::fatal("[window_manager::create_instance_impl] Render API '{}' is not implemented", info.api);
        return nullptr;
    }
} // engine