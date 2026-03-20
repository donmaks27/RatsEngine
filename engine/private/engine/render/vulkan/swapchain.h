#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <EASTL/span.h>
#include <glm/vec2.hpp>

namespace engine::vulkan
{
	class swapchain final : _value_wrapper<vk::SwapchainKHR>
	{
	public:
		swapchain() = default;
		swapchain(std::nullptr_t) {}
		swapchain(const swapchain&) = delete;
		swapchain(swapchain&&) noexcept = default;
		~swapchain() { clear(); }

		swapchain& operator=(const swapchain&) = delete;
		swapchain& operator=(swapchain&&) noexcept = default;
		swapchain& operator=(std::nullptr_t)
		{
			clear();
			return *this;
		}

		struct create_info
		{
			vk::SurfaceKHR surface = nullptr;
			glm::uvec2 surfaceSize = { 0, 0 };
			bool disableVSync = false;
		};
		[[nodiscard]] bool init(const context& ctx, const create_info& createInfo);

		void clear();
		void clear(const context& ctx);

		[[nodiscard]] bool outdated() const { return valid() && m_outdated; }
		[[nodiscard]] bool acquire_next_image(const context& ctx, const vk::Semaphore& imageAvailable);
		[[nodiscard]] bool present(const context& ctx, eastl::span<const vk::Semaphore> waitSemaphores);

		[[nodiscard]] vk::Image image() const { return m_images.size() > m_currentImageIndex ? m_images[m_currentImageIndex] : nullptr; }
		[[nodiscard]] vk::ImageView image_view() const { return m_imageViews.size() > m_currentImageIndex ? m_imageViews[m_currentImageIndex] : nullptr; }

	private:

		eastl::vector<vk::Image> m_images;
		eastl::vector<vk::ImageView> m_imageViews;

		std::uint8_t m_currentImageIndex = std::numeric_limits<std::uint8_t>::max();
		bool m_outdated = false;
	};
}
