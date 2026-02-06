#pragma once

#include <engine/core.h>
#include <engine/render/window_manager.h>

struct GLFWwindow;

namespace engine
{
    class window_manager_glfw : public window_manager
    {
        using super = window_manager;

    protected:
        window_manager_glfw() = default;
        virtual ~window_manager_glfw() override = default;
    public:

        [[nodiscard]] virtual bool should_close_window(const window_id& id) const override;

        virtual void on_frame_end() override;

    protected:

        eastl::vector_map<window_id, GLFWwindow*> m_windowDataGLFW;

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

        virtual bool create_window_impl(const window_id& id, const window_create_info& info) override;

    private:

        void clear_GLFW();
    };
}