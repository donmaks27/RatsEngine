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

        log::log("[window_manager::create_instance] Creating instance of window manager ({})...", info.api);
        s_instance = create_instance_impl(info);
        if (s_instance == nullptr)
        {
            log::fatal("[window_manager::create_instance] Failed to create instance of window manager!");
            return nullptr;
        }
        if (!s_instance->init(info))
        {
            log::fatal("[window_manager::create_instance] Failed to initialize window manager instance!");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            return nullptr;
        }
        log::info("[window_manager::create_instance] Window manager instance created successfully");
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

    bool window_manager::init(const create_info& info)
    {
        m_mainWindowId = window_id::generate();
        m_windowData.emplace(m_mainWindowId, window_data{});
        if (!create_window_impl(m_mainWindowId, {}))
        {
            log::fatal("[window_manager::init] Failed to create main window!");
            return false;
        }
        return true;
    }

    void window_manager::clear()
    {
        m_windowData.clear();
        m_mainWindowId = window_id::invalid_id();
    }
}
