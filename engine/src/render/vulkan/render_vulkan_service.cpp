#include <engine/render/vulkan/render_vulkan_service.h>

#include <engine/engine.h>
#include <engine/render/window_service.h>
#include <engine/render/vulkan/surface_vulkan_service.h>
#include <engine/render/vulkan/builder/instance_builder.h>
#include <engine/render/vulkan/builder/device_builder.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace engine
{
    namespace
	{
    	constexpr eastl::array RequiredInstanceExtensions = { vk::KHRGetSurfaceCapabilities2ExtensionName };
    	constexpr eastl::array RequiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
		constexpr auto MaxInstanceVulkanApiVersion = vk::ApiVersion13;
		constexpr auto MinDeviceVulkanApiVersion = vk::ApiVersion11;
	}

	RATS_ENGINE_SERVICE_IMPL(render_vulkan_service)

	render_service* render_service::instance_allocate_vulkan() { return new render_vulkan_service(); }

	bool render_vulkan_service::service_init(const service_create_info_t& info)
	{
		if (!super_t::service_init(info))
		{
			return false;
		}

		auto& event_bus = engine::instance().events();
		if (!create_instance(info))
		{
			Log.fatal("Failed to create Vulkan instance!");
			return false;
		}
		if (!event_bus.post_immediate<vulkan_instance_created_event>({}))
		{
			return false;
		}
    	if (!create_device())
    	{
			Log.fatal("Failed to create Vulkan device!");
    		return false;
    	}
		if (!event_bus.post_immediate<vulkan_device_created_event>({}))
		{
			return false;
		}
		if (!create_command_pools())
		{
			Log.fatal("Failed to create Vulkan command pools!");
			return false;
		}
    	if (!create_frame_data())
    	{
    		Log.fatal("Failed to create Vulkan frame data!");
    		return false;
    	}
		return true;
	}

	void render_vulkan_service::service_clear()
	{
		if (m_ctx.m_instance != nullptr)
		{
			const auto& device = m_ctx.d();
			device->waitIdle();

			for (auto& frame : m_framesInFlight)
			{
				frame.commandPool.clear(m_ctx);
				frame.commandBuffer = nullptr;

				device->destroyFence(frame.frameFence);
				device->destroySemaphore(frame.imageAvailableSemaphore);
				frame.imageAvailableSemaphore = nullptr;
				frame.frameFence = nullptr;

				frame.available = true;
			}

			m_transferCommandPool.clear(m_ctx);

			surface_service::instance()->clear_surfaces();
            m_ctx.m_device.clear();
			m_ctx.m_instance.clear();
		}

		super_t::service_clear();
	}

	bool render_vulkan_service::create_instance(const service_create_info_t& info)
	{
		auto instance = vulkan::instance_builder()
			.set_application_name(info.appName)
			.set_engine_name("RatsEngine")
			.set_application_version(0, 1, 0)
			.set_engine_version(0, 1, 0)
			.add_required_extensions(surface_vulkan_service::required_instance_extensions())
			.add_required_extensions(RequiredInstanceExtensions)
			.set_max_vulkan_version(MaxInstanceVulkanApiVersion)
			.build();
		if (instance == nullptr)
		{
			return false;
		}
		m_ctx.m_instance = std::move(instance);
		return true;
	}

	bool render_vulkan_service::create_device()
    {
    	const auto mainSurface = surface_vulkan_service::instance()->surface(window_service::instance()->main_window_id());
        auto device = vulkan::device_builder()
            .add_required_extensions(RequiredDeviceExtensions)
            .set_min_vulkan_version(MinDeviceVulkanApiVersion)
			.feature(vulkan::device_feature::sampler_anisotropy, vulkan::device_feature_type::required)
            .feature(vulkan::device_feature::dynamic_render, vulkan::device_feature_type::required)
			.feature(vulkan::device_feature::synchronization2, vulkan::device_feature_type::required)
            .collect_physical_devices(m_ctx.m_instance, mainSurface)
            .build(m_ctx.m_instance);
        if (device == nullptr)
        {
            return false;
        }
        m_ctx.m_device = std::move(device);
        return true;
	}

	bool render_vulkan_service::create_command_pools()
	{
		m_transferCommandPool = m_ctx.d().queue(vulkan::queue_type::transfer).command_pool(m_ctx,
			vk::CommandPoolCreateFlagBits::eTransient);
		return m_transferCommandPool.valid();
	}

	bool render_vulkan_service::create_frame_data()
	{
    	const auto& device = m_ctx.d();
    	for (auto& frame : m_framesInFlight)
    	{
    		const auto fenceResult = device->createFence({});
    		if (fenceResult.result != vk::Result::eSuccess)
    		{
    			Log.fatal("Failed to create frameFence: {}", fenceResult.result);
    			return false;
    		}
    		frame.frameFence = fenceResult.value;

    		auto semaphoreResult = device->createSemaphore({});
    		if (semaphoreResult.result != vk::Result::eSuccess)
    		{
    			Log.fatal("Failed to create imageAvailableSemaphore: {}", semaphoreResult.result);
    			return false;
    		}
    		frame.imageAvailableSemaphore = semaphoreResult.value;

    		frame.commandPool = device.queue(vulkan::queue_type::graphics).command_pool(m_ctx);
    		if (!frame.commandPool.valid())
    		{
    			Log.fatal("Failed to create command pool for frame");
    			return false;
    		}
    		frame.commandBuffer = frame.commandPool.allocate(m_ctx);
    	}
    	return true;
	}

	bool render_vulkan_service::render()
	{
		auto windowService = window_service::instance();
		auto surfaceService = surface_vulkan_service::instance();
		const auto mainSurfaceId = windowService->main_window_id();
		const auto& device = m_ctx.d();
		const auto swapchainSize = surfaceService->surface_size(mainSurfaceId);

    	if (!surfaceService->recreate_outdated_swapchains(m_ctx))
    	{
    		return false;
    	}

		prepare_next_frame();
		auto& frameData = m_framesInFlight[m_currentFrameIndex];

    	static eastl::vector<surface_id> enabledSurfaces;
    	enabledSurfaces.clear();
    	if (!acquire_swapchain_images(enabledSurfaces))
    	{
    		return false;
    	}
    	if (enabledSurfaces.empty())
    	{
    		Log.log("No surfaces are enabled");
    		return true;
    	}
    	auto& swapchain = surfaceService->surface_swapchain(enabledSurfaces.front());
    	const auto& imageData = swapchain.image();

		const auto& cmd = frameData.commandBuffer;
    	frameData.commandPool.reset(m_ctx);
		cmd.begin(vk::CommandBufferBeginInfo{});

		// Render
		vk::ImageMemoryBarrier2 barrier;
		barrier.image = imageData.image;
		barrier.subresourceRange = vk::ImageSubresourceRange{ 
			vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
		};
		barrier.srcStageMask = vk::PipelineStageFlagBits2::eNone;
		barrier.srcAccessMask = vk::AccessFlagBits2::eNone;
		barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		const vk::DependencyInfo dependencyInfo{ {}, {}, {}, { barrier } };
		cmd.pipelineBarrier2(dependencyInfo);

		const auto colorAttachmentInfo = vk::RenderingAttachmentInfo()
			.setImageView(imageData.imageView)
			.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setClearValue({vk::ClearColorValue{ 1.0f, 0.3f, 0.0f, 1.0f }});
		const auto renderingInfo = vk::RenderingInfo()
			.setColorAttachments({ colorAttachmentInfo })
			.setRenderArea({ {}, { swapchainSize.x, swapchainSize.y } })
			.setLayerCount(1);
		cmd.beginRendering(renderingInfo);
		cmd.endRendering();

		barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
		barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
		barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
		cmd.pipelineBarrier2(dependencyInfo);

		cmd.end();

		const vk::SemaphoreSubmitInfo waitSemaphoreInfo{ frameData.imageAvailableSemaphore, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput };
		const vk::SemaphoreSubmitInfo signalSemaphoreInfo{ imageData.renderFinishedSemaphore, 0, vk::PipelineStageFlagBits2::eAllGraphics };
		const vk::CommandBufferSubmitInfo cmdInfo{ cmd };
		const auto submitInfo = vk::SubmitInfo2()
			.setWaitSemaphoreInfos({ waitSemaphoreInfo })
			.setSignalSemaphoreInfos({ signalSemaphoreInfo })
			.setCommandBufferInfos({ cmdInfo });
		device.queue(vulkan::queue_type::graphics)->submit2({ submitInfo }, frameData.frameFence);
    	frameData.available = false;

    	if (!present_swapchains(enabledSurfaces))
    	{
    		return false;
    	}
		return true;
	}
	void render_vulkan_service::prepare_next_frame()
    {
    	m_currentFrameIndex = (m_currentFrameIndex + 1) % m_framesInFlight.size();
    	auto& frameData = m_framesInFlight[m_currentFrameIndex];
    	if (!frameData.available)
    	{
    		m_ctx.d()->waitForFences({ frameData.frameFence }, vk::True, std::numeric_limits<std::uint64_t>::max());
    		m_ctx.d()->resetFences({ frameData.frameFence });
    		frameData.available = true;
    	}
    }
	bool render_vulkan_service::acquire_swapchain_images(eastl::vector<surface_id>& enabledSurfaces)
	{
    	auto& surfaceService = *surface_vulkan_service::instance();
    	auto& frameData = m_framesInFlight[m_currentFrameIndex];
    	for (const auto id : surfaceService.surface_ids())
    	{
			auto& swapchain = surfaceService.surface_swapchain(id);
    		if (!swapchain.valid() || swapchain.outdated())
    		{
    			continue;
    		}

    		const auto [acquireResult, swapchainImageIndex] = m_ctx.d()->acquireNextImage2KHR({
    			*swapchain, std::numeric_limits<std::uint64_t>::max(),
    			frameData.imageAvailableSemaphore, nullptr, 1
    		});
    		if (acquireResult == vk::Result::eErrorOutOfDateKHR)
    		{
    			Log.log("Swapchain for surface {} is out of date", id);
    			swapchain.mark_outdated();
    			continue;
    		}
    		if (acquireResult == vk::Result::eSuboptimalKHR)
    		{
    			Log.log("Swapchain for surface {} is suboptimal", id);
    		}
    		else if (acquireResult != vk::Result::eSuccess)
    		{
    			Log.error("Failed to acquire next swapchain image for surface {}: {}", id, acquireResult);
    			return false;
    		}

    		swapchain.set_image_index(swapchainImageIndex);
    		enabledSurfaces.push_back(id);
    	}
    	return true;
	}
	bool render_vulkan_service::present_swapchains(const eastl::vector<surface_id>& enabledSurfaces) const
	{
    	auto& surfaceService = *surface_vulkan_service::instance();
    	const auto& queue = m_ctx.d().queue(vulkan::queue_type::present);
    	for (const auto surfaceId : enabledSurfaces)
    	{
    		auto& swapchain = surfaceService.surface_swapchain(surfaceId);
    		const std::uint32_t swapchainImageIndex = swapchain.image_index();
    		const auto result = queue->presentKHR({
    			{ swapchain.image().renderFinishedSemaphore },
    			{ *swapchain },
    			{ swapchainImageIndex }
    		});
    		if (result == vk::Result::eErrorOutOfDateKHR)
    		{
    			Log.log("Swapchain for surface {} is out of date", surfaceId);
    			swapchain.mark_outdated();
    		}
    		else if (result == vk::Result::eSuboptimalKHR)
    		{
    			Log.log("Swapchain for surface {} is suboptimal", surfaceId);
    		}
    		else if (result != vk::Result::eSuccess)
    		{
    			Log.error("Failed to present image for surface {}: {}", surfaceId, result);
    			return false;
    		}
    	}
    	return true;
	}
}
