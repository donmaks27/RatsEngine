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
        log::log("[window_manager::init] Creating main window {}...", m_mainWindowId);
        if (!create_window_impl(m_mainWindowId, {}))
        {
            log::fatal("[window_manager::init] Failed to create main window!");
            m_windowData = {};
            return false;
        }
        log::info("[window_manager::init] Main window created successfully");
        return true;
    }

    void window_manager::clear()
    {
        m_windowData.clear();
        m_mainWindowId = window_id::invalid_id();
    }

    window_id window_manager::create_window(const window_create_info& info)
    {
        window_id id = window_id::generate();
        while (m_windowData.find(id) != m_windowData.end())
        {
            id = window_id::generate();
        }

        log::log("[window_manager::create_window] Creating window {}...", id);
        m_windowData.emplace(id, window_data{});
        if (!create_window_impl(m_mainWindowId, {}))
        {
            log::error("[window_manager::create_window] Failed to create window!");
            m_windowData.erase(id);
            return window_id::invalid_id();
        }
        log::log("[window_manager::create_window] Window created successfully");
        return id;
    }
    bool window_manager::destroy_window(const window_id& id)
    {
        if (m_windowData.count(id) == 0)
        {
            return false;
        }
        if (id == m_mainWindowId)
        {
            log::warning("[window_manager::destroy_window] Can't destroy main window");
            return false;
        }
        log::log("[window_manager::destroy_window] Destroying window {}...", id);
        destroy_window_impl(id);
        m_windowData.erase(id);
        log::log("[window_manager::destroy_window] Window destroyed");
        return true;
    }
}
