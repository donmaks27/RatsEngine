#pragma once

#include <engine/core.h>
#include <engine/render/surface_service.h>

namespace engine
{
    class window1_service : public surface_backend_service
    {
	public:
        using super = surface_backend_service;

    protected:
        window1_service() { Instance = this; }
        virtual ~window1_service() override { Instance = nullptr; }
    public:

        [[nodiscard]] static constexpr log::logger logger() { return { "window", super::super::logger() }; }
        inline static const log::logger Log = logger();
        [[nodiscard]] static auto instance() { return Instance; }

    private:

        static window1_service* Instance;
    };
}
