#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <glm/vec2.hpp>

namespace engine::vulkan
{
    class swapchain_builder final
    {
    public:
        swapchain_builder() = default;
        swapchain_builder(const swapchain_builder&) = delete;
        swapchain_builder(swapchain_builder&&) = delete;
        ~swapchain_builder() = default;

        swapchain_builder& operator=(const swapchain_builder&) = delete;
        swapchain_builder& operator=(swapchain_builder&&) = delete;

        [[nodiscard]] swapchain_builder& set_surface(vk::SurfaceKHR surface);
        [[nodiscard]] swapchain_builder& set_size(glm::uvec2 size);
        [[nodiscard]] swapchain_builder& disable_vsync(bool disable = true);

        [[nodiscard]] swapchain build(const context& ctx);
        [[nodiscard]] bool build(const context& ctx, swapchain& prevSwapchain);

    private:

        vk::SurfaceKHR m_surface = nullptr;
        glm::uvec2 m_size = { 0, 0 };
        bool m_disableVSync = false;
    };
}
