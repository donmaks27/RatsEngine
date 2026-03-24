#pragma once

#include <engine/core.h>
#include <engine/system.h>

#include <engine/render/render_api.h>

namespace engine
{
    struct render_system_create_info
    {
        std::string appName = "RatsEngine";

        render_api api = render_api::vulkan;
    };

    class RATS_ENGINE_EXPORT render_system : public system<render_system, render_system_create_info>
    {
        friend system;

    protected:
        render_system() = default;
        virtual ~render_system() override = default;
    public:
        render_system(const render_system&) = delete;
        render_system(render_system&&) = delete;

        render_system& operator=(const render_system&) = delete;
        render_system& operator=(render_system&&) = delete;

        [[nodiscard]] static constexpr log::logger logger() { return log::logger("render_system", logger_engine()); }

        [[nodiscard]] virtual bool render() = 0;

    protected:

        [[nodiscard]] virtual bool system_init(const instance_create_info& info) override;
        virtual void system_clear() override;

    private:

        static const log::logger Log;
        static render_system* Instance;
        static render_system* instance_allocate(const instance_create_info& info);
    };
}