#include <engine/render/render_manager.h>

#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
    #include <engine/render/vulkan/render_manager_vulkan.h>
#endif

namespace engine
{
    render_manager* render_manager::create_instance_impl(const create_info& info)
    {
        switch (info.api)
        {
#if RATS_ENGINE_VULKAN_ENABLE
        case render_api::vulkan: return new vulkan::render_manager();
#endif
        default:;
        }
        log::fatal("[render_manager::create_instance_impl] Render API '{}' is not implemented", info.api);
        return nullptr;
    }
}