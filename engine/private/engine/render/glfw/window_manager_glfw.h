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

        [[nodiscard]] virtual bool shouldCloseMainWindow() const override;
        virtual void on_frame_end() override;

    protected:

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

    private:

        GLFWwindow* m_mainWindow = nullptr;


        void clear_GLFW();
    };
}