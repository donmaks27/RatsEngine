module;

#include <EASTL/vector.h>

export module rats_engine.render.glfw.vulkan;

import rats_engine.render.glfw;
import rats_engine.render.vulkan;

export namespace engine
{
    class window_manager_glfw_vulkan : public window_manager_glfw, public window_manager_vulkan
    {
        using super = window_manager_glfw;

    public:
        window_manager_glfw_vulkan() = default;
        virtual ~window_manager_glfw_vulkan() override = default;

        [[nodiscard]] virtual eastl::vector<const char*> get_required_extensions() const override;
    };
}