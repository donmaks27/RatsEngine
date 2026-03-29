#pragma once

#include <engine/core.h>
#include <engine/service.h>

#include <engine/render/render_api.h>

namespace engine
{
    template<typename CreateInfo>
    concept render_service_create_info = requires(CreateInfo info) {
        { info.renderApi } -> std::convertible_to<render_api>;
    };

    struct default_render_service_create_info
    {
        render_api renderApi = render_api::vulkan;
    };
    template<typename T, render_service_create_info CreateInfo = default_render_service_create_info>
    class render_service_of : public service_of<T, CreateInfo>
    {
        using super = service_of<T, CreateInfo>;
        friend super;
        friend super::super;

        static T* instance_allocate(const super::service_create_info& info)
        {
            T* result = nullptr;
            switch (info.renderApi)
            {
            case render_api::vulkan: result = T::instance_allocate_vulkan(); break;
            default:;
            }
            if (result == nullptr)
            {
                T::logger().fatal("Render API '{}' is not implemented", info.renderApi);
            }
            return result;
        }
    };
}