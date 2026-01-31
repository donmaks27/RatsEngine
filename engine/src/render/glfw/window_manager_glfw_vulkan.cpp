module;

module rats_engine.render.glfw.vulkan;

import rats_engine.render;

namespace engine
{
    window_manager* window_manager::create_instance_impl_vulkan(const create_info&)
    {
        return new window_manager_glfw_vulkan();
    }
}