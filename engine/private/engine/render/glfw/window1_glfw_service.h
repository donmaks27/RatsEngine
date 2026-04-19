#pragma once

#include <engine/core.h>
#include <engine/render/window1_service.h>

#include <GLFW/glfw3.h>

namespace engine
{
    class window1_glfw_service : public window1_service
    {
        RATS_ENGINE_SERVICE(window1_glfw_service, "GLFW")

    public:

        [[nodiscard]] GLFWwindow* glfw_window(surface_id id) const;

    protected:

        virtual bool service_init(const render_api_service_create_info& info) override;
        virtual void service_clear() override;

        [[nodiscard]] virtual int glfw_client_api() const { return GLFW_NO_API; }

        virtual bool create_window_impl(surface_id id, const window_create_info& info) override;
        virtual void destroy_window_impl(surface_id id) override;

    private:

        struct window_data_GLFW
        {
            GLFWwindow* window = nullptr;
        };
        eastl::vector_map<surface_id, window_data_GLFW> m_windowDataGLFW;
    };
}
