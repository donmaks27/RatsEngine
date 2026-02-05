#include <engine/render/render_manager.h>

#if RATS_ENGINE_RENDER_VULKAN
    #include <engine/render/vulkan/render_manager_vulkan.h>
#endif

namespace engine
{
    render_manager* render_manager::create_instance_impl(const create_info& info)
    {
        switch (info.api)
        {
#if RATS_ENGINE_RENDER_VULKAN
        case render_api::vulkan: return new vulkan::render_manager();
#endif
        default:;
        }
        log::fatal("[render_manager::create_instance_impl] Render API '{}' is not implemented", info.api);
        return nullptr;
    }
}