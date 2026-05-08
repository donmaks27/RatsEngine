#include <engine/render/vulkan/builder/swapchain_builder.h>

#include <engine/render/render_api.h>

#include <EASTL/span.h>

namespace engine::vulkan
{
	namespace
	{
		const auto Log = log::logger("swapchain_builder", logger_render_api<render_api::vulkan>());

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

    swapchain_builder& swapchain_builder::set_surface(const vk::SurfaceKHR surface)
    {
        m_surface = surface;
        return *this;
    }
    swapchain_builder& swapchain_builder::set_size(const glm::uvec2 size)
    {
        m_size = size;
        return *this;
    }
    swapchain_builder& swapchain_builder::disable_vsync(const bool disable)
    {
        m_disableVSync = disable;
        return *this;
    }

    swapchain swapchain_builder::build(const context& ctx)
    {
        swapchain result;
        if (!build(ctx, result))
        {
            return nullptr;
        }
        return result;
    }
    bool swapchain_builder::build(const context& ctx, swapchain& prevSwapchain)
    {
    	if (m_surface == nullptr)
    	{
    		Log.warning("Surface is null");
    		return false;
    	}

		const auto& device = ctx.d();
		const auto& physicalDevice = device.physical_device();
		const auto& presentQueue = device.queue(queue_type::present);
		if (physicalDevice.getSurfaceSupportKHR(presentQueue.family_index(), m_surface).value != vk::True)
		{
			Log.warning("Unsupported surface for current present queue");
			return true;
		}

		const auto surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(m_surface).value;
		auto surfaceFormats = physicalDevice.getSurfaceFormats2KHR({ m_surface }).value;
		const auto surfaceCaps = physicalDevice.getSurfaceCapabilities2KHR({ m_surface }).value.surfaceCapabilities;

		const auto selectedPresentMode = pickPresentMode(surfacePresentModes, m_disableVSync);
		const auto selectedFormat = std::ranges::min_element(surfaceFormats, std::less(), [](const vk::SurfaceFormat2KHR& format) {
			return getSurfaceFormatPriority(format.surfaceFormat);
		})->surfaceFormat;
		const auto swapchainExtent = pickSwapchainExtent(surfaceCaps, m_size);
		const std::uint32_t swapchainImageCount = surfaceCaps.maxImageCount > 0 ?
			std::clamp(surfaceCaps.minImageCount + 1, surfaceCaps.minImageCount, surfaceCaps.maxImageCount) :
			(surfaceCaps.minImageCount + 1);

		vk::SwapchainCreateInfoKHR swapchainCreateInfo{ {},
			m_surface,
			swapchainImageCount,
			selectedFormat.format, selectedFormat.colorSpace,
			swapchainExtent, 1,
			vk::ImageUsageFlagBits::eColorAttachment,
			vk::SharingMode::eExclusive, {},
			surfaceCaps.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque,
			selectedPresentMode,
			vk::True,
			*prevSwapchain
		};
		const auto& graphicsQueue = device.queue(queue_type::graphics);
		const std::uint32_t queueFamilies[] = { graphicsQueue.family_index(), presentQueue.family_index() };
		if (graphicsQueue.family_index() != presentQueue.family_index())
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilies;
		}

		const auto [swapchainResult, swapchain] = device->createSwapchainKHR(swapchainCreateInfo);
		if (swapchainResult != vk::Result::eSuccess)
		{
			Log.error("Failed to create swapchain: {}", swapchainResult);
			prevSwapchain.clear();
			return false;
		}
		const auto [swapchainImagesResult, swapchainImages] = device->getSwapchainImagesKHR(swapchain);
		if (swapchainImagesResult != vk::Result::eSuccess)
		{
			Log.error("Failed to get swapchain images: {}", swapchainImagesResult);
			prevSwapchain.clear();
			device->destroySwapchainKHR(swapchain);
			return false;
		}

		device->destroySwapchainKHR(*prevSwapchain);
		for (std::size_t index = 0; index < prevSwapchain.m_images.size(); ++index)
		{
			auto& data = prevSwapchain.m_images[index];
			data.image = nullptr;
			device->destroyImageView(data.imageView);
			if (index >= swapchainImages.size())
			{
				device->destroySemaphore(data.renderFinishedSemaphore);
			}
		}

		prevSwapchain.m_value = swapchain;
		prevSwapchain.m_images.resize(swapchainImages.size());
		prevSwapchain.m_imageIndex = std::numeric_limits<std::uint8_t>::max();
		prevSwapchain.m_outdated = false;
		for (std::size_t index = 0; index < prevSwapchain.m_images.size(); ++index)
		{
			auto& data = prevSwapchain.m_images[index];
			data.image = swapchainImages[index];

			const auto [imageViewResult, imageView] = device->createImageView({ {},
				swapchainImages[index], vk::ImageViewType::e2D, selectedFormat.format, {}, {
					vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
				} });
			if (imageViewResult != vk::Result::eSuccess)
			{
				Log.error("Failed to create view for swapchain image {}: {}", index, imageViewResult);
				prevSwapchain.clear();
				return false;
			}
			data.imageView = imageView;

			if (data.renderFinishedSemaphore == nullptr)
			{
				const auto [semaphoreResult, semaphore] = device->createSemaphore({});
				if (semaphoreResult != vk::Result::eSuccess)
				{
					Log.error("Failed to create semaphore for swapchain image {}: {}", index, semaphoreResult);
					prevSwapchain.clear();
					return false;
				}
				data.renderFinishedSemaphore = semaphore;
			}
		}
        return true;
    }
}
