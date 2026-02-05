#include <engine/render/window_manager.h>

#ifdef RATS_ENGINE_WINDOW_GLFW

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
        case render_api::vulkan: return nullptr;
        default:;
        }
        log::fatal("[window_manager::create_instance_impl] Render API '{}' is not implemented", info.api);
        return nullptr;
#endif
    }
}