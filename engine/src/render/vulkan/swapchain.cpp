#include <engine/render/vulkan/swapchain.h>

#include <engine/render/vulkan/render_manager_vulkan.h>

#include <EASTL/span.h>

namespace engine::vulkan
{
	namespace 
	{
		[[nodiscard]] vk::PresentModeKHR pickPresentMode(const eastl::span<const vk::PresentModeKHR> modes, const bool preferDisableVSync)
		{
			if (preferDisableVSync)
			{
				const auto iter = std::ranges::find(modes, vk::PresentModeKHR::eImmediate);
				if (iter != modes.end())
				{
					return *iter;
				}
			}
			const auto iter = std::ranges::find(modes, vk::PresentModeKHR::eMailbox);
			return iter != modes.end() ? *iter : vk::PresentModeKHR::eFifo;
		}
		[[nodiscard]] std::uint8_t getSurfaceFormatPriority(const vk::SurfaceFormatKHR format)
		{
			std::uint8_t priority = 0;
			if ((format.format == vk::Format::eB8G8R8A8Srgb) && (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear))
			{
				return priority;
			}
			priority++;
			if ((format.format == vk::Format::eR8G8B8A8Srgb) && (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear))
			{
				return priority;
			}
			priority++;
			if ((format.format == vk::Format::eB8G8R8A8Unorm) && (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear))
			{
				return priority;
			}
			priority++;
			if ((format.format == vk::Format::eR8G8B8A8Unorm) && (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear))
			{
				return priority;
			}
			return std::numeric_limits<decltype(priority)>::max();
		}
		[[nodiscard]] vk::Extent2D pickSwapchainExtent(const vk::SurfaceCapabilitiesKHR& caps, const glm::uvec2& desired)
		{
			if (caps.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
			{
				return caps.currentExtent;
			}
			return {
				std::clamp(desired.x, caps.minImageExtent.width, caps.maxImageExtent.width),
				std::clamp(desired.y, caps.minImageExtent.height, caps.maxImageExtent.height)
			};
		}
	}

	bool swapchain::init(const context& ctx, const create_info& createInfo)
	{
		if (createInfo.surface == nullptr)
		{
			log::warning("[vulkan::swapchain::init] Surface is null");
			clear();
			return false;
		}

		const auto& device = ctx.d();
		const auto& physicalDevice = device.physical_device();
		const auto& presentQueue = device.queue(queue_type::present);
		if (physicalDevice.getSurfaceSupportKHR(presentQueue.family_index(), createInfo.surface).value != vk::True)
		{
			log::warning("[vulkan::swapchain::init] Unsupported surface for current present queue");
			clear();
			return true;
		}

		const auto surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(createInfo.surface).value;
		auto surfaceFormats = physicalDevice.getSurfaceFormats2KHR({ createInfo.surface }).value;
		const auto surfaceCaps = physicalDevice.getSurfaceCapabilities2KHR({ createInfo.surface }).value.surfaceCapabilities;

		const auto selectedPresentMode = pickPresentMode(surfacePresentModes, createInfo.disableVSync);
		const auto selectedFormat = std::ranges::min_element(surfaceFormats, std::less(), [](const vk::SurfaceFormat2KHR& format) {
			return getSurfaceFormatPriority(format.surfaceFormat);
		})->surfaceFormat;
		const auto swapchainExtent = pickSwapchainExtent(surfaceCaps, createInfo.surfaceSize);
		const std::uint32_t swapchainImageCount = surfaceCaps.maxImageCount > 0 ? 
			std::clamp(surfaceCaps.minImageCount + 1, surfaceCaps.minImageCount, surfaceCaps.maxImageCount) : 
			(surfaceCaps.minImageCount + 1);

		vk::SwapchainCreateInfoKHR swapchainCreateInfo{ {},
			createInfo.surface, 
			swapchainImageCount,
			selectedFormat.format, selectedFormat.colorSpace,
			swapchainExtent, 1,
			vk::ImageUsageFlagBits::eColorAttachment, 
			vk::SharingMode::eExclusive, {},
			surfaceCaps.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, 
			selectedPresentMode,
			vk::True,
			m_swapchain
		};
		const auto swapchainValue = device->createSwapchainKHR(swapchainCreateInfo);
		if (m_swapchain != nullptr)
		{
			device->destroySwapchainKHR(m_swapchain);
			m_swapchain = nullptr;
		}
		if (swapchainValue.result != vk::Result::eSuccess)
		{
			log::error("[vulkan::swapchain::init] Failed to create swapchain: {}", swapchainValue.result);
			clear();
			return false;
		}

		m_swapchain = swapchainValue.value;
		const auto swapchainImages = device->getSwapchainImagesKHR(swapchainValue.value);
		if (swapchainImages.result != vk::Result::eSuccess)
		{
			log::error("[vulkan::swapchain::init] Failed to get swapchain images: {}", swapchainImages.result);
			clear();
			return false;
		}

		m_images.resize(swapchainImages.value.size());
		std::ranges::copy(swapchainImages.value, m_images.begin());
		return true;
	}

	void swapchain::clear()
	{
		clear(render_manager_vulkan::instance()->vk_ctx());
	}
	void swapchain::clear(const context& ctx)
	{
		m_images.clear();
		if (valid())
		{
			ctx.d()->destroySwapchainKHR(m_swapchain);
			m_swapchain = nullptr;
		}
	}
}
