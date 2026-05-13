#include <engine/render/render_service.h>

#include <engine/core_engine.h>
#include <engine/render/surface_service.h>

namespace engine
{
    RATS_ENGINE_BASE_SERVICE_IMPL(render_service)

#if !RATS_ENGINE_VULKAN_ENABLE
    render_service* allocate_render_service_vulkan() { return nullptr; }
#endif

    render_service* allocate_render_service()
    {
        const auto& cfg = core_engine::instance().config();
        switch (cfg.renderApi)
        {
        case render_api::vulkan: return allocate_render_service_vulkan();
        default: ;
        }
        return nullptr;
    }

    bool render_service::service_init()
    {
        return true;
    }

    void render_service::service_clear()
    {
    }
}
