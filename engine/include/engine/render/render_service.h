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

    class RATS_ENGINE_EXPORT render_service : public service_of<render_service, render_service_instance_create_info>
    {
        using super = service_of;

    protected:
        render_service() { Instance = this; }
        virtual ~render_service() override { Instance = nullptr; }
    public:

        [[nodiscard]] static constexpr auto logger() { return log::logger("render", super::logger()); }
        [[nodiscard]] static auto instance() { return Instance; }
        [[nodiscard]] static render_service* instance_allocate_vulkan();

        [[nodiscard]] virtual bool render() = 0;

    protected:

        virtual bool service_init(const service_create_info& info) override;
        virtual void service_clear() override;

    private:

        static const log::logger Log;
        static render_service* Instance;
    };
}