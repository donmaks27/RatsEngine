#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/render_service.h>
#include <engine/events.h>

#include <EASTL/array.h>

namespace engine
{
    struct vulkan_instance_created_event final : event_of<vulkan_instance_created_event> {};
    struct vulkan_device_created_event final : event_of<vulkan_device_created_event> {};

    class render_vulkan_service final : public render_service
    {
        RATS_ENGINE_SERVICE(render_vulkan_service, render_api_to_string(render_api::vulkan))

    public:

        [[nodiscard]] const vulkan::context& vk_ctx() const { return m_ctx; }

        virtual bool render() override;

    protected:

        virtual bool service_init(const service_create_info_t& info) override;
        virtual void service_clear() override;

    private:

        struct frame_data
        {
            vulkan::command_pool commandPool = nullptr;
            vk::CommandBuffer commandBuffer = nullptr;

            vk::Fence frameFence = nullptr;
            vk::Semaphore imageAvailableSemaphore = nullptr;

            bool available = true;
        };

        vulkan::context m_ctx;
        vulkan::command_pool m_transferCommandPool;

        eastl::array<frame_data, 2> m_framesInFlight;
        std::uint8_t m_currentFrameInFlight = 0;

        [[nodiscard]] bool create_instance(const service_create_info_t& info);
        [[nodiscard]] bool create_device();
        [[nodiscard]] bool create_command_pools();
        [[nodiscard]] bool create_frame_data();
    };
}
