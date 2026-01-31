module;

export module rats_engine.render.glfw.vulkan;

import rats_engine.render.glfw;

export namespace engine
{
    class window_manager_glfw_vulkan : public window_manager_glfw
    {
        using super = window_manager_glfw;

    public:
        window_manager_glfw_vulkan() = default;
        virtual ~window_manager_glfw_vulkan() override = default;
    };
}