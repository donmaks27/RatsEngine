#include <engine/render/render_system.h>

#include <engine/private_config_macro.h>

#if RATS_ENGINE_VULKAN_ENABLE
    #include <engine/render/vulkan/render_system_vulkan.h>
#endif

namespace engine
{
	namespace
	{
        const auto& Log = render_system::logger();
	}

    render_system* render_system::create_instance_impl(const create_info& info)
    {
        switch (info.api)
        {
#if RATS_ENGINE_VULKAN_ENABLE
        case render_api::vulkan: return new render_system_vulkan();
#endif
        default:;
        }
        Log.fatal("create_instance_impl: Render API '{}' is not implemented", info.api);
        return nullptr;
    }
}