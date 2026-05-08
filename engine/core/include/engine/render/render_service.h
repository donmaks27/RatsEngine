#pragma once

#include <engine/core.h>
#include <engine/service.h>

namespace engine
{
    struct render_service_instance_create_info
    {
        std::string appName = "RatsEngine";
        render_api renderApi = render_api::vulkan;
    };

    class RATS_ENGINE_EXPORT render_service : public service_of<render_service>
    {
        RATS_ENGINE_SERVICE_BASE(render_service, "render")

    public:

        [[nodiscard]] virtual bool render() = 0;

    protected:

        virtual bool service_init() override;
        virtual void service_clear() override;

    private:

        [[nodiscard]] static render_service* instance_allocate_vulkan();
    };
}