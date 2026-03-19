#pragma once

#include <engine/core.h>

#include <engine/render/render_api.h>

namespace engine
{
    class RATS_ENGINE_EXPORT render_system
    {
    protected:
        render_system() = default;
        virtual ~render_system() = default;
    public:
        render_system(const render_system&) = delete;
        render_system(render_system&&) = delete;

        render_system& operator=(const render_system&) = delete;
        render_system& operator=(render_system&&) = delete;

        [[nodiscard]] static constexpr log::logger logger() { return log::logger("render_system", logger_engine()); }

        struct create_info
        {
            std::string appName = "RatsEngine";

            render_api api = render_api::vulkan;
        };
        static render_system* create_instance(const create_info& info);
        [[nodiscard]] static render_system* instance() { return s_instance; }
        static void clear_instance();

    protected:

        [[nodiscard]] virtual bool init(const create_info& info);
        virtual void clear();

    private:

        static const log::logger Log;

        static render_system* s_instance;
        static render_system* create_instance_impl(const create_info& info);
    };
}