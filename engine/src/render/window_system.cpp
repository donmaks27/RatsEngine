#include <engine/render/window_system.h>
#include <engine/engine.h>

namespace engine
{
    const log::logger window_system::Log = window_system::logger();
    window_system* window_system::Instance = nullptr;

    window_system::window_system()
    {
        engine::instance().event_bus().add_listener(this);
    }
    window_system::~window_system()
    {
        engine::instance().event_bus().remove_listener(this);
    }

    bool window_system::system_init(const instance_create_info& info)
    {
        m_mainWindowId = window_id::generate();
        m_windowData.emplace(m_mainWindowId, window_data{ .size = info.mainWindow.size });
        Log.log("Creating main window {}...", m_mainWindowId);
        if (!create_window_impl(m_mainWindowId, info.mainWindow))
        {
            Log.fatal("Failed to create main window!");
            m_windowData = {};
            return false;
        }
        Log.info("Main window created successfully");
        return true;
    }
    void window_system::system_clear()
    {
        m_windowData.clear();
        m_mainWindowId = window_id::invalid_id();
    }

    glm::uvec2 window_system::window_size(const window_id& id) const
    {
        const auto iter = m_windowData.find(id);
		return iter != m_windowData.end() ? iter->second.size : glm::uvec2{ 0, 0 };
    }

    window_id window_system::create_window(const window_create_info& info)
    {
        window_id id = window_id::generate();
        while (m_windowData.find(id) != m_windowData.end())
        {
            id = window_id::generate();
        }

        Log.log("Creating window {}...", id);
        m_windowData.emplace(id, window_data{ .size = info.size });
        if (!create_window_impl(m_mainWindowId, { .size = info.size }))
        {
            Log.error("Failed to create window!");
            m_windowData.erase(id);
            return window_id::invalid_id();
        }
        Log.log("Window created successfully");
        return id;
    }
    bool window_system::destroy_window(const window_id& id)
    {
        if (m_windowData.count(id) == 0)
        {
            return false;
        }
        if (id == m_mainWindowId)
        {
            Log.warning("Can't destroy main window");
            return false;
        }
        Log.log("Destroying window {}...", id);
        destroy_window_impl(id);
        m_windowData.erase(id);
        Log.log("Window destroyed");
        return true;
    }
}
