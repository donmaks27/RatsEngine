#include <engine/engine.h>

#include <engine/render/render_manager.h>
#include <engine/render/window_manager.h>
#include <engine/utils/macro/defer.h>

namespace engine
{
    engine::~engine()
    {
        if (is_started())
        {
            clear_engine();
        }
    }

    bool engine::start()
    {
        if (is_started())
        {
            log::error("[engine::start] Engine already started!");
            return false;
        }
        m_engineStarted = true;

        RATS_ENGINE_DEFER([this] { clear_engine(); });

        log::log("[engine::start] Initializing engine...");
        if (!init_engine())
        {
            log::fatal("[engine::start] Engine initialization failed!");
            return false;
        }
        log::info("[engine::start] Engine initialized successfully");

        log::log("[engine::start] Game loop started");
        auto* windowManager = window_manager::instance();
        while (!windowManager->should_close_main_window())
        {
            windowManager->on_frame_end();
        }
        log::log("[engine::start] Game loop stopped");
        return true;
    }

    bool engine::init_engine()
    {
        if (render_manager::create_instance({ .api = render_api::vulkan }) == nullptr)
        {
            return false;
        }
        return true;
    }

    void engine::clear_engine()
    {
        log::log("[engine::clear_engine] Clearing engine...");

        render_manager::clear_instance();

        log::info("[engine::clear_engine] Engine cleared successfully");
        m_engineStarted = false;
    }
}