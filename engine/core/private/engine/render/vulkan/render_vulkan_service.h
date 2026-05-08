#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/events.h>
#include <engine/render/render_service.h>
#include <engine/render/surface_id.h>

#include <EASTL/array.h>

namespace engine
{
    struct vulkan_instance_created_event final : event_of<vulkan_instance_created_event> {};
    struct vulkan_device_created_event final : event_of<vulkan_device_created_event> {};

    class render_vulkan_service final : public render_service
    {
        friend render_service* allocate_render_service_vulkan();

        RATS_ENGINE_SERVICE(render_vulkan_service, render_api_to_string(render_api::vulkan))

    public:

        [[nodiscard]] const vulkan::context& vk_ctx() const { return m_ctx; }

        virtual bool render() override;

    protected:

        virtual bool service_init() override;
        virtual void service_clear() override;

    private:

        struct frame_swapchain_data
        {
            vk::CommandBuffer commandBuffer = nullptr;
            vk::Semaphore imageAvailableSemaphore = nullptr;

            surface_id surfaceId = invalid_surface_id;
        };
        struct frame_data
        {
            vk::Fence frameFence = nullptr;
            vulkan::command_pool commandPool = nullptr;

            eastl::vector<frame_swapchain_data> swapchainData;

            bool available = true;
        };

        vulkan::context m_ctx;
        vulkan::command_pool m_transferCommandPool;

        eastl::array<frame_data, 2> m_framesInFlight;
        std::uint8_t m_currentFrameIndex = 0;

        [[nodiscard]] bool create_instance();
        [[nodiscard]] bool create_device();
        [[nodiscard]] bool create_command_pools();
        [[nodiscard]] bool create_frame_data();

        [[nodiscard]] bool prepare_next_frame();
        [[nodiscard]] bool allocate_frame_swapchain_data(surface_id requestedIndex, surface_id surfaceId);
        [[nodiscard]] bool acquire_swapchain_image(surface_id surfaceId, vulkan::swapchain& swapchain,
            const frame_swapchain_data& frameSwapchainData) const;
        [[nodiscard]] static bool present_swapchain(surface_id surfaceId, vulkan::swapchain& swapchain,
            const vulkan::queue& queue);
    };
}
