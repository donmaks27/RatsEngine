#pragma once

#include <engine/core.h>
#include <engine/render/window_system.h>

struct GLFWwindow;

namespace engine
{
    class window_system_glfw : public window_system
    {
        using super = window_system;

    protected:
        window_system_glfw() = default;
        virtual ~window_system_glfw() override = default;
    public:

        [[nodiscard]] virtual bool should_close_window(const window_id& id) const override;

        virtual void on_frame_end() override;

    protected:

        eastl::vector_map<window_id, GLFWwindow*> m_windowDataGLFW;

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

        virtual bool create_window_impl(const window_id& id, const window_create_info& info) override;
        virtual void destroy_window_impl(const window_id& id) override;

    private:

        void clear_GLFW();
    };
}