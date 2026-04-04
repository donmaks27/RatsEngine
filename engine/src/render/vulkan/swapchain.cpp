#include <engine/render/vulkan/swapchain.h>

#include <engine/render/vulkan/render_service_vulkan.h>

#include <EASTL/span.h>

namespace engine::vulkan
{
	namespace 
	{
		const auto Log = log::logger("swapchain", logger_vulkan());

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
			Log.warning("Surface is null");
			clear();
			return false;
		}

		const auto& device = ctx.d();
		const auto& physicalDevice = device.physical_device();
		const auto& presentQueue = device.queue(queue_type::present);
		if (physicalDevice.getSurfaceSupportKHR(presentQueue.family_index(), createInfo.surface).value != vk::True)
		{
			Log.warning("Unsupported surface for current present queue");
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

		const auto& graphicsQueue = device.queue(queue_type::graphics);
		const std::uint32_t queueFamilies[] = { graphicsQueue.family_index(), presentQueue.family_index() };
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
			m_value
		};
		if (graphicsQueue.family_index() != presentQueue.family_index())
		{
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilies;
		}
		const auto swapchainValue = device->createSwapchainKHR(swapchainCreateInfo);
		clear();
		if (swapchainValue.result != vk::Result::eSuccess)
		{
			Log.error("Failed to create swapchain: {}", swapchainValue.result);
			return false;
		}

		m_value = swapchainValue.value;
		const auto swapchainImages = device->getSwapchainImagesKHR(swapchainValue.value);
		if (swapchainImages.result != vk::Result::eSuccess)
		{
			Log.error("Failed to get swapchain images: {}", swapchainImages.result);
			clear();
			return false;
		}

		m_images.reserve(swapchainImages.value.size());
		const bool imagesValid = std::ranges::all_of(swapchainImages.value, [this, &device, format = selectedFormat.format](const vk::Image& image) {
			auto& data = m_images.emplace_back();
			data.image = image;

			const auto imageViewResult = device->createImageView({ {},
				image, vk::ImageViewType::e2D, format, {}, {
					vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
				} });
			if (imageViewResult.result != vk::Result::eSuccess)
			{
				Log.error("Failed to create view for swapchain image: {}", imageViewResult.result);
				return false;
			}
			data.imageView = imageViewResult.value;

			const auto semaphoreResult = device->createSemaphore({});
			if (semaphoreResult.result != vk::Result::eSuccess)
			{
				Log.error("Failed to create semaphore for swapchain image: {}", semaphoreResult.result);
				return false;
			}
			data.renderFinishedSemaphore = semaphoreResult.value;
			return true;
		});
		if (!imagesValid)
		{
			clear();
			return false;
		}
		return true;
	}

	void swapchain::clear()
	{
		clear(render_service_vulkan::instance()->vk_ctx());
	}
	void swapchain::clear(const context& ctx)
	{
		if (valid())
		{
			const auto& device = ctx.d();
			device->waitIdle();

			m_outdated = false;
			std::ranges::for_each(m_images, [&device](const image_data& data) {
				device->destroyImageView(data.imageView);
				device->destroySemaphore(data.renderFinishedSemaphore);
			});
			m_images.clear();
			device->destroySwapchainKHR(m_value);
			m_value = nullptr;
		}
	}

	bool swapchain::acquire_next_image(const context& ctx, const vk::Semaphore& imageAvailable)
	{
		if (!valid())
		{
			Log.warning("Swapchain is not valid");
			return false;
		}
		if (outdated())
		{
			Log.warning("Swapchain is outdated");
			return true;
		}

		const auto acquireResult = ctx.d()->acquireNextImage2KHR(
		{
			m_value, std::numeric_limits<std::uint64_t>::max(), imageAvailable, nullptr, 1
		});
		if (acquireResult.result != vk::Result::eSuccess)
		{
			switch (acquireResult.result)
			{
			case vk::Result::eSuboptimalKHR:
				Log.log("Swapchain is suboptimal");
				break;
			case vk::Result::eErrorOutOfDateKHR:
				Log.log("Swapchain is out of date");
				m_outdated = true;
				break;

			default:
				Log.error("Failed to acquire next image: {}", acquireResult.result);
				return false;
			}

			return true;
		}

		m_currentImageIndex = static_cast<std::uint8_t>(acquireResult.value);
		return true;
	}
	bool swapchain::present(const context& ctx, const vk::Semaphore& waitSemaphore)
	{
		if (!valid())
		{
			Log.warning("Swapchain is not valid");
			return false;
		}
		if (outdated())
		{
			Log.warning("Swapchain is outdated");
			return true;
		}
		if (m_currentImageIndex >= m_images.size())
		{
			Log.warning("No image acquired");
			return false;
		}
		
		const auto imageIndex = static_cast<std::uint32_t>(m_currentImageIndex);
		const auto result = ctx.d().queue(queue_type::present)->presentKHR({
			{ waitSemaphore }, { m_value }, { imageIndex }
		});
		if (result != vk::Result::eSuccess)
		{
			switch (result)
			{
			case vk::Result::eSuboptimalKHR:
				Log.log("Swapchain is suboptimal");
				break;
			case vk::Result::eErrorOutOfDateKHR:
				Log.log("Swapchain is out of date");
				m_outdated = true;
				break;
			default:
				Log.error("Failed to present image: {}", result);
				return false;
			}
		}
		return true;
	}
}
