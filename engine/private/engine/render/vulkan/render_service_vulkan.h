#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/render_service.h>
#include <engine/utils/events.h>

#include "EASTL/array.h"

namespace engine
{
    struct vulkan_instance_created_event final : utils::event_of<vulkan_instance_created_event> {};
    struct vulkan_device_created_event final : utils::event_of<vulkan_device_created_event> {};

    class render_service_vulkan final : public render_service
    {
        using super = render_service;

    public:
        render_service_vulkan() { Instance = this; }
        virtual ~render_service_vulkan() override { Instance = nullptr; }

        [[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(super::logger()); }
        [[nodiscard]] static render_service_vulkan* instance() { return Instance; }

        [[nodiscard]] const vulkan::context& vk_ctx() const { return m_ctx; }

        virtual bool render() override;

    protected:

        virtual bool service_init(const service_create_info& info) override;
        virtual void service_clear() override;

    private:

        static const log::logger Log;
        static render_service_vulkan* Instance;

        struct frame_data
        {
            vk::Semaphore imageAvailableSemaphore = nullptr;
            vk::Fence frameFence = nullptr;

            vk::CommandBuffer commandBuffer = nullptr;
        };

        vulkan::context m_ctx;
        vulkan::command_pool m_graphicsCommandPool;
        vulkan::command_pool m_transferCommandPool;

        eastl::array<frame_data, 2> m_framesInFlight;
        std::uint8_t m_currentFrameInFlight = 0;

        [[nodiscard]] bool create_instance(const service_create_info& info);
        [[nodiscard]] bool create_device();
        [[nodiscard]] bool create_command_pools();
    };
}
