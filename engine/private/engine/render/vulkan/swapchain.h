#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <glm/vec2.hpp>

namespace engine::vulkan
{
	class swapchain final
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

		[[nodiscard]] const vk::SwapchainKHR* operator->() const { return &m_swapchain; }
		[[nodiscard]] const vk::SwapchainKHR& operator*() const { return m_swapchain; }

		[[nodiscard]] bool valid() const { return m_swapchain != nullptr; }
		[[nodiscard]] bool operator!=(std::nullptr_t) const { return valid(); }
		[[nodiscard]] bool operator==(std::nullptr_t) const { return !valid(); }

		struct create_info
		{
			vk::SurfaceKHR surface = nullptr;
			glm::uvec2 surfaceSize = { 0, 0 };
			bool preferDisableVSync = false;
		};
		[[nodiscard]] bool init(const context& ctx, const create_info& createInfo);

		void clear();

	private:

		vk::SwapchainKHR m_swapchain = nullptr;
		eastl::vector<vk::Image> m_swapchainImages;
	};
}
