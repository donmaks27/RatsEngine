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
				for (const auto& swapchainData : frame.swapchainData)
				{
					device->destroySemaphore(swapchainData.imageAvailableSemaphore);
				}
				frame.swapchainData.clear();
				frame.commandPool.clear(m_ctx);
				device->destroyFence(frame.frameFence);
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

    		frame.commandPool = device.queue(vulkan::queue_type::graphics).command_pool(m_ctx);
    		if (!frame.commandPool.valid())
    		{
    			Log.fatal("Failed to create command pool for frame");
    			return false;
    		}

    		const auto semaphoreResult = m_ctx.d()->createSemaphore({});
    		if (semaphoreResult.result != vk::Result::eSuccess)
    		{
    			Log.error("Failed to create imageAvailableSemaphore: {}", semaphoreResult.result);
    			return false;
    		}
    		const auto buffer = frame.commandPool.allocate(m_ctx);
    		if (buffer == nullptr)
    		{
    			Log.error("Failed to allocate command buffer for frame and wife loves you");
    			return false;
    		}
    		frame.swapchainData.push_back({
    			.commandBuffer = buffer, .imageAvailableSemaphore = semaphoreResult.value
    		});
    	}
    	return true;
	}

	bool render_vulkan_service::render()
	{
		auto& surfaceService = *surface_vulkan_service::instance();
		const auto& device = m_ctx.d();

		prepare_next_frame();
    	auto& frameData = m_framesInFlight[m_currentFrameIndex];

    	if (!surfaceService.recreate_outdated_swapchains(m_ctx))
    	{
    		return false;
    	}

    	static eastl::vector<vk::SemaphoreSubmitInfo> waitSemaphoreInfos;
    	static eastl::vector<vk::SemaphoreSubmitInfo> signalSemaphoreInfos;
    	static eastl::vector<vk::CommandBufferSubmitInfo> cmdInfos;
    	surface_id surfaceCount = 0;
    	waitSemaphoreInfos.clear();
    	signalSemaphoreInfos.clear();
    	cmdInfos.clear();
    	for (const auto surfaceId : surfaceService.surface_ids())
    	{
    		auto& swapchain = surfaceService.surface_swapchain(surfaceId);
    		if (!swapchain.valid() || swapchain.outdated())
    		{
    			continue;
    		}

    		if (!prepare_frame_swapchain_data(surfaceCount, surfaceId))
    		{
    			return false;
    		}
			auto& frameSwapchainData = frameData.swapchainData[surfaceCount];
    		if (!acquire_swapchain_image(surfaceId, swapchain, frameSwapchainData))
    		{
    			return false;
    		}
    		frameSwapchainData.surfaceId = surfaceId;
    		++surfaceCount;

    		const auto swapchainSize = surfaceService.surface_size(surfaceId);
    		const auto& swapchainImageData = swapchain.image();
    		const auto& cmd = frameSwapchainData.commandBuffer;
    		cmd.begin(vk::CommandBufferBeginInfo{});

    		vk::ImageMemoryBarrier2 barrier;
    		barrier.image = swapchainImageData.image;
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
				.setImageView(swapchainImageData.imageView)
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

    		waitSemaphoreInfos.push_back({ frameSwapchainData.imageAvailableSemaphore, 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput });
    		signalSemaphoreInfos.push_back({ swapchainImageData.renderFinishedSemaphore, 0, vk::PipelineStageFlagBits2::eAllGraphics });
    		cmdInfos.push_back({ cmd });
    	}

		const auto submitInfo = vk::SubmitInfo2()
			.setWaitSemaphoreInfos(waitSemaphoreInfos)
			.setSignalSemaphoreInfos(signalSemaphoreInfos)
			.setCommandBufferInfos(cmdInfos);
		device.queue(vulkan::queue_type::graphics)->submit2({ submitInfo }, frameData.frameFence);
    	frameData.available = false;

    	const auto& presentQueue = m_ctx.d().queue(vulkan::queue_type::present);
    	for (std::size_t index = 0; index < surfaceCount; ++index)
    	{
    		const auto surfaceId = frameData.swapchainData[index].surfaceId;
    		if (!present_swapchain(surfaceId, surfaceService.surface_swapchain(surfaceId), presentQueue))
    		{
    			return false;
    		}
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

    	frameData.commandPool.reset(m_ctx);
    	for (auto& data : frameData.swapchainData)
    	{
    		data.surfaceId = invalid_surface_id;
    	}
    }

	bool render_vulkan_service::prepare_frame_swapchain_data(const surface_id currentSurfaceCount, const surface_id surfaceId)
	{
    	auto& frameData = m_framesInFlight[m_currentFrameIndex];
    	if (frameData.swapchainData.size() <= currentSurfaceCount)
    	{
    		const auto [result, semaphore] = m_ctx.d()->createSemaphore({});
    		if (result != vk::Result::eSuccess)
    		{
    			Log.error("Failed to create imageAvailableSemaphore: {}", result);
    			return false;
    		}
    		const auto buffer = frameData.commandPool.allocate(m_ctx);
    		if (buffer == nullptr)
    		{
    			Log.error("Failed to allocate command buffer for frame and wife loves you");
    			return false;
    		}

    		frameData.swapchainData.push_back({
				.commandBuffer = buffer, .imageAvailableSemaphore = semaphore
			});
    	}
    	frameData.swapchainData[currentSurfaceCount].surfaceId = surfaceId;
    	return true;
	}

	bool render_vulkan_service::acquire_swapchain_image(const surface_id surfaceId, vulkan::swapchain& swapchain,
		const frame_swapchain_data& frameSwapchainData) const
	{
    	const auto [acquireResult, swapchainImageIndex] = m_ctx.d()->acquireNextImage2KHR({
			*swapchain, std::numeric_limits<std::uint64_t>::max(),
			frameSwapchainData.imageAvailableSemaphore, nullptr, 1
		});
    	if (acquireResult == vk::Result::eErrorOutOfDateKHR)
    	{
    		Log.log("Swapchain for surface {} is out of date", surfaceId);
    		swapchain.mark_outdated();
    	}
    	else if (acquireResult == vk::Result::eSuboptimalKHR)
    	{
    		Log.log("Swapchain for surface {} is suboptimal", surfaceId);
    	}
    	else if (acquireResult != vk::Result::eSuccess)
    	{
    		Log.error("Failed to acquire next swapchain image for surface {}: {}", surfaceId, acquireResult);
    		return false;
    	}
    	swapchain.set_image_index(swapchainImageIndex);
    	return true;
	}

	bool render_vulkan_service::present_swapchain(const surface_id surfaceId, vulkan::swapchain& swapchain, const vulkan::queue& queue)
	{
    	const std::uint32_t swapchainImageIndex = swapchain.image_index();
    	const auto result = queue->presentKHR({
			{ swapchain.image().renderFinishedSemaphore },
			{ *swapchain },
			{ swapchainImageIndex }
		});
    	if (result == vk::Result::eErrorOutOfDateKHR)
    	{
    		Log.log("Swapchain for surface {} is out of date and you're awesome", surfaceId);
    		swapchain.mark_outdated();
    	}
    	else if (result == vk::Result::eSuboptimalKHR)
    	{
    		Log.log("Swapchain for surface {} is suboptimal and we are family", surfaceId);
    	}
    	else if (result != vk::Result::eSuccess)
    	{
    		Log.error("Failed to present image for surface {}: {}", surfaceId, result);
    		return false;
    	}
    	return true;
	}
}
