#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/render_system.h>
#include <engine/utils/events.h>

#include "EASTL/array.h"

namespace engine
{
    struct vulkan_instance_created_event final : utils::event<vulkan_instance_created_event> {};
    struct vulkan_device_created_event final : utils::event<vulkan_device_created_event> {};

    class render_system_vulkan final : public render_system
    {
        using super = render_system;

    public:
        render_system_vulkan();
        virtual ~render_system_vulkan() override;

        [[nodiscard]] static constexpr log::logger logger() { return vulkan::logger_vulkan(super::logger()); }
        [[nodiscard]] static render_system_vulkan* instance() { return Instance; }

        [[nodiscard]] const vulkan::context& vk_ctx() const { return m_ctx; }

        virtual bool render() override;

    protected:

        virtual bool system_init(const instance_create_info& info) override;
        virtual void system_clear() override;

    private:

        static const log::logger Log;
        static render_system_vulkan* Instance;

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

        [[nodiscard]] bool create_instance(const instance_create_info& info);
        [[nodiscard]] bool create_device();
        [[nodiscard]] bool create_command_pools();
    };
}
