#pragma once

#include <engine/core.h>
#include <engine/service.h>

#include <engine/render/render_api.h>

namespace engine
{
    class RATS_ENGINE_EXPORT render_service : public service
    {
        RATS_ENGINE_BASE_SERVICE(render_service, "render")

    public:

        [[nodiscard]] virtual bool render() = 0;

    protected:

        virtual bool service_init() override;
        virtual void service_clear() override;
    };

    [[nodiscard]] render_service* allocate_render_service_vulkan();
    [[nodiscard]] render_service* allocate_render_service(render_api renderApi);
}
