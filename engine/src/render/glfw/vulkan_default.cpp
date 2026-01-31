module rats_engine.render;

import rats_engine.utils;

namespace engine
{
    window_manager* window_manager::create_instance_impl_vulkan(const create_info&)
    {
        log::error("[window_manager::create_instance_impl_vulkan] Render API '{}' is not supported", render_api::vulkan);
        return nullptr;
    }
}