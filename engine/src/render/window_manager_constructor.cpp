#include <engine/render/window_manager.h>

#include <engine/private_config_macro.h>

#undef RATS_ENGINE_WINDOW_NONE
#undef RATS_ENGINE_WINDOW_MANAGER_IMPL_VULKAN

#if RATS_ENGINE_GLFW_ENABLE
    #if RATS_ENGINE_VULKAN_ENABLE
        #include <engine/render/glfw/window_manager_glfw_vulkan.h>
        #define RATS_ENGINE_WINDOW_MANAGER_IMPL_VULKAN engine::vulkan::window_manager_glfw
    #endif
#else
    #define RATS_ENGINE_WINDOW_NONE
#endif

namespace engine
{
    window_manager* window_manager::create_instance_impl(const create_info& info)
    {
#ifdef RATS_ENGINE_WINDOW_NONE
        log::fatal("[window_manager::create_instance_impl] Can't find any implementation of window manager!");
        return nullptr;
#else
        switch (info.api)
        {
#ifdef RATS_ENGINE_WINDOW_MANAGER_IMPL_VULKAN
        case render_api::vulkan: return new RATS_ENGINE_WINDOW_MANAGER_IMPL_VULKAN();
#endif
        default:;
        }
        log::fatal("[window_manager::create_instance_impl] Render API '{}' is not implemented", info.api);
        return nullptr;
#endif
    }
}