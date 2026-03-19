#include <engine/render/window_system.h>
#include <engine/engine.h>

namespace engine
{
    const log::logger& window_system::logger()
    {
        static const log::logger logger("window_system", engine::logger());
        return logger;
    }
    namespace
    {
        const auto& Log = window_system::logger();
    }

    window_system* window_system::s_instance = nullptr;

    window_system::window_system()
    {
        engine::instance().event_bus().add_listener(this);
    }
    window_system::~window_system()
    {
        engine::instance().event_bus().remove_listener(this);
    }

    window_system* window_system::create_instance(const create_info& info)
    {
        if (s_instance != nullptr)
        {
            return s_instance;
        }

        Log.log("create_instance: Creating instance of window manager ({})...", info.api);
        s_instance = create_instance_impl(info);
        if (s_instance == nullptr)
        {
            Log.fatal("create_instance: Failed to create instance of window manager!");
            return nullptr;
        }
        if (!s_instance->init(info))
        {
            Log.fatal("create_instance: Failed to initialize window manager instance!");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            return nullptr;
        }
        Log.info("create_instance: Window manager instance created successfully");
        return s_instance;
    }
    void window_system::clear_instance()
    {
        if (s_instance != nullptr)
        {
            Log.log("clear_instance: Clearing instance of window manager...");
            s_instance->clear();
            delete s_instance;
            s_instance = nullptr;
            Log.log("clear_instance: Window manager instance cleared successfully");
        }
    }

    bool window_system::init(const create_info& info)
    {
        m_mainWindowId = window_id::generate();
        m_windowData.emplace(m_mainWindowId, window_data{});
        Log.log("init: Creating main window {}...", m_mainWindowId);
        if (!create_window_impl(m_mainWindowId, {}))
        {
            Log.fatal("init: Failed to create main window!");
            m_windowData = {};
            return false;
        }
        Log.info("init: Main window created successfully");
        return true;
    }

    void window_system::clear()
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

        Log.log("create_window: Creating window {}...", id);
        m_windowData.emplace(id, window_data{ .size = info.size });
        if (!create_window_impl(m_mainWindowId, { .size = info.size }))
        {
            Log.error("create_window: Failed to create window!");
            m_windowData.erase(id);
            return window_id::invalid_id();
        }
        Log.log("create_window: Window created successfully");
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
            Log.warning("destroy_window: Can't destroy main window");
            return false;
        }
        Log.log("destroy_window: Destroying window {}...", id);
        destroy_window_impl(id);
        m_windowData.erase(id);
        Log.log("destroy_window: Window destroyed");
        return true;
    }
}
