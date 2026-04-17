#pragma once

#include <engine/core.h>
#include <engine/render/window_service.h>

struct GLFWwindow;

namespace engine
{
    class window_service_glfw : public window_service
    {
        RATS_ENGINE_SERVICE(window_service_glfw, "GLFW")

    public:

        [[nodiscard]] virtual bool should_close_window(const window_id& id) const override;

        virtual void on_frame_end() override;

    protected:

        eastl::vector_map<window_id, GLFWwindow*> m_windowDataGLFW;

        virtual bool service_init(const service_create_info_t& info) override;
        virtual void service_clear() override;

        virtual bool create_window_impl(const window_id& id, const window_create_info& info) override;
        virtual void destroy_window_impl(const window_id& id) override;

    private:

        void clear_GLFW();
    };
}