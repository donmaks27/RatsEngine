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

		for (auto& frame : m_framesInFlight)
		{
			auto semaphoreResult = m_ctx.d()->createSemaphore({});
			if (semaphoreResult.result != vk::Result::eSuccess)
			{
				Log.fatal("Failed to create imageAvailableSemaphore: {}", semaphoreResult.result);
				return false;
			}
			frame.imageAvailableSemaphore = semaphoreResult.value;

			const auto fenceResult = m_ctx.d()->createFence({ vk::FenceCreateFlagBits::eSignaled });
			if (fenceResult.result != vk::Result::eSuccess)
			{
				Log.fatal("Failed to create frameFence: {}", fenceResult.result);
				return false;
			}
			frame.frameFence = fenceResult.value;

			frame.commandBuffer = m_graphicsCommandPool.command_buffer(m_ctx);
		}

		return true;
	}

	void render_vulkan_service::service_clear()
	{
		if (m_ctx.m_instance != nullptr)
		{
			m_ctx.d()->waitIdle();

			for (auto& frame : m_framesInFlight)
			{
				m_ctx.d()->destroySemaphore(frame.imageAvailableSemaphore);
				m_ctx.d()->destroyFence(frame.frameFence);
				frame.imageAvailableSemaphore = nullptr;
				frame.frameFence = nullptr;
			}

			m_transferCommandPool.clear(m_ctx);
			m_graphicsCommandPool.clear(m_ctx);

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
			.add_required_extensions(surface_vulkan_service::instance()->required_instance_extensions())
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
		m_graphicsCommandPool = m_ctx.d().queue(vulkan::queue_type::graphics).command_pool(m_ctx,
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		m_transferCommandPool = m_ctx.d().queue(vulkan::queue_type::transfer).command_pool(m_ctx,
			vk::CommandPoolCreateFlagBits::eTransient);
		return m_graphicsCommandPool.valid() && m_transferCommandPool.valid();
	}

	bool render_vulkan_service::render()
	{
		auto windowService = window_service::instance();
		auto surfaceService = surface_vulkan_service::instance();
		const auto mainSurfaceId = windowService->main_window_id();
		const auto& device = m_ctx.d();
		const auto swapchainSize = surfaceService->surface_size(mainSurfaceId);
		auto& swapchain = surfaceService->surface_swapchain(mainSurfaceId);

		m_currentFrameInFlight = (m_currentFrameInFlight + 1) % m_framesInFlight.size();
		const auto& frameData = m_framesInFlight[m_currentFrameInFlight];
		const auto& cmd = frameData.commandBuffer;

		device->waitForFences({ frameData.frameFence }, vk::True, std::numeric_limits<std::uint64_t>::max());

		if (!swapchain.acquire_next_image(m_ctx, frameData.imageAvailableSemaphore))
		{
			return false;
		}
		if (swapchain.outdated())
		{
			return swapchain.init(m_ctx, { surfaceService->surface(mainSurfaceId), swapchainSize });
		}
		device->resetFences({ frameData.frameFence });

		cmd.reset();
		cmd.begin(vk::CommandBufferBeginInfo{});

		// Render
		vk::ImageMemoryBarrier2 barrier;
		barrier.image = swapchain.image();
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
			.setImageView(swapchain.image_view())
			.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setClearValue({vk::ClearColorValue{ 1.0f, 0.2f, 0.3f, 1.0f }});
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

		const auto renderFinishedSemaphore = swapchain.render_finished_semaphore();
		const vk::SemaphoreSubmitInfo waitSemaphoreInfo{ frameData.imageAvailableSemaphore , 0, vk::PipelineStageFlagBits2::eColorAttachmentOutput };
		const vk::SemaphoreSubmitInfo signalSemaphoreInfo{ renderFinishedSemaphore , 0, vk::PipelineStageFlagBits2::eAllGraphics };
		const vk::CommandBufferSubmitInfo cmdInfo{ cmd };
		const auto submitInfo = vk::SubmitInfo2()
			.setWaitSemaphoreInfos({ waitSemaphoreInfo })
			.setSignalSemaphoreInfos({ signalSemaphoreInfo })
			.setCommandBufferInfos({ cmdInfo });
		device.queue(vulkan::queue_type::graphics)->submit2({ submitInfo }, frameData.frameFence);
		if (!swapchain.present(m_ctx, renderFinishedSemaphore))
		{
			return false;
		}

		return true;
	}
}
