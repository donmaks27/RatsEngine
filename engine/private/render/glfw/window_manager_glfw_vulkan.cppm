module;

#include <EASTL/vector.h>

export module rats_engine.render.glfw.vulkan;

import rats_engine.render.glfw;
import rats_engine.render.vulkan;

export namespace engine::vulkan
{
    class window_manager_glfw : public engine::window_manager_glfw, public engine::vulkan::window_manager
    {
        using super = engine::window_manager_glfw;

    public:
        window_manager_glfw() = default;
        virtual ~window_manager_glfw() override = default;

        [[nodiscard]] virtual eastl::vector<const char*> get_required_extensions() const override;
    };
}