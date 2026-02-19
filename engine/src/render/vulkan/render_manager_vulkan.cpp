#include <engine/render/vulkan/render_manager_vulkan.h>

#include <engine/render/window_manager.h>
#include <engine/render/vulkan/builder/instance_builder.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace engine
{
    namespace
	{
    	constexpr eastl::array RequiredInstanceExtensions = { vk::KHRGetSurfaceCapabilities2ExtensionName };
    	constexpr eastl::array RequiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
    	constexpr auto MinVulkanApiVersion = vk::ApiVersion11;

		enum class device_feature_type : std::uint8_t { none, extension, core };
    	struct physical_device_queue_data
    	{
    		std::uint32_t familyIndex = 0;
    		std::uint32_t queueIndex = 0;
    	};
    	struct physical_device_data
		{
			vk::PhysicalDevice device;
			vk::PhysicalDeviceProperties properties;
    		eastl::vector<vk::QueueFamilyProperties2> queueProperties;
			vk::DeviceSize VRAM = 0;
			std::uint32_t score = 0;

    		physical_device_queue_data graphicsQueue;
    		physical_device_queue_data transferQueue;

    		device_feature_type dynamicRendering = device_feature_type::none;
		};
    	physical_device_data calculate_physical_device_score(const vk::PhysicalDevice& device, const vk::SurfaceKHR& mainSurface)
		{
    		physical_device_data result{ .device = device };

			const auto& properties = result.properties = device.getProperties2().properties;
			if (properties.apiVersion < MinVulkanApiVersion)
			{
				return result;
			}

    		eastl::vector<const char*> requiredExtensions(RequiredDeviceExtensions.begin(), RequiredDeviceExtensions.end());
    		vk::PhysicalDeviceFeatures2 features;
    		// Core features
    		vk::PhysicalDeviceVulkan13Features features13;
    		// Extension features
    		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures;
    		void** nextFeature = &features.pNext;
    		if (properties.apiVersion >= vk::ApiVersion13)
    		{
    			*nextFeature = &features13;
    			nextFeature = &features13.pNext;

    			result.dynamicRendering = device_feature_type::core;
    		}
    		else
    		{
    			requiredExtensions.push_back(vk::KHRDynamicRenderingExtensionName);
    			*nextFeature = &dynamicRenderingFeatures;
    			nextFeature = &dynamicRenderingFeatures.pNext;

    			result.dynamicRendering = device_feature_type::extension;
    		}

			// Check device extensions
			const bool hasAllExtensions = std::ranges::all_of(requiredExtensions, [extensions = device.enumerateDeviceExtensionProperties().value](const char* requiredExtension) {
				return std::ranges::any_of(extensions, [&requiredExtension](const vk::ExtensionProperties& extension) {
					return std::strcmp(requiredExtension, extension.extensionName) == 0;
				});
			});
			if (!hasAllExtensions)
			{
				return result;
			}

    		// Check queue families
    		std::ranges::copy(device.getQueueFamilyProperties2(), std::back_inserter(result.queueProperties));
    		auto graphicsIter = std::ranges::find_if(result.queueProperties, [](const vk::QueueFamilyProperties2& queueFamily) {
				return !!(queueFamily.queueFamilyProperties.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer));
			});
    		if (graphicsIter != result.queueProperties.end())
    		{
    			result.graphicsQueue.familyIndex = static_cast<std::uint32_t>(std::distance(result.queueProperties.begin(), graphicsIter));
    			result.graphicsQueue.queueIndex = 0;
    			result.transferQueue.familyIndex = result.graphicsQueue.familyIndex;
    			const auto queueCount = result.queueProperties[result.graphicsQueue.familyIndex].queueFamilyProperties.queueCount;
    			result.transferQueue.queueIndex = queueCount >= 2 ? 1 : 0;
    		}
    		else
    		{
    			graphicsIter = std::ranges::find_if(result.queueProperties, [](const vk::QueueFamilyProperties2& queueFamily) {
					return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
				});
    			const auto transferIter = std::ranges::find_if(graphicsIter + 1, result.queueProperties.end(), [](const vk::QueueFamilyProperties2& queueFamily) {
					return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer);
				});
    			if ((graphicsIter == result.queueProperties.end()) || (transferIter == result.queueProperties.end()))
    			{
    				return result;
    			}

    			result.graphicsQueue.familyIndex = static_cast<std::uint32_t>(std::distance(result.queueProperties.begin(), graphicsIter));
    			result.graphicsQueue.queueIndex = 0;
    			result.transferQueue.familyIndex = static_cast<std::uint32_t>(std::distance(result.queueProperties.begin(), transferIter));
    			result.transferQueue.queueIndex = 0;
    		}

			// Check device features
			device.getFeatures2(&features);
			const bool dynamicRenderingSupported = properties.apiVersion >= vk::ApiVersion13 ? features13.dynamicRendering :dynamicRenderingFeatures.dynamicRendering;
			const bool anisotropySupported = features.features.samplerAnisotropy;
			if (!dynamicRenderingSupported || !anisotropySupported)
			{
				return result;
			}

			// Check surface support
			if (device.getSurfacePresentModesKHR(mainSurface).value.empty())
			{
				return result;
			}
			if (device.getSurfaceFormats2KHR({ mainSurface }).value.empty())
			{
				return result;
			}

    		const auto& memoryProperties = device.getMemoryProperties2().memoryProperties;
    		const auto iter = std::ranges::find_if(memoryProperties.memoryHeaps, [](const vk::MemoryHeap& heap) {
    			return !!(heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal);
    		});
    		if (iter == memoryProperties.memoryHeaps.end())
    		{
    			return result;
    		}
    		result.VRAM = iter->size;

    		switch (properties.deviceType)
    		{
		    case vk::PhysicalDeviceType::eDiscreteGpu:   result.score += 1000; break;
		    case vk::PhysicalDeviceType::eIntegratedGpu: result.score += 500;  break;
		    default: ;
    		}

			return result;
		}

    	vk::ResultValue<vk::Device> create_device(const physical_device_data& data)
    	{
    		eastl::vector<const char*> extensions(RequiredDeviceExtensions.begin(), RequiredDeviceExtensions.end());
    		vk::PhysicalDeviceFeatures deviceFeatures{};
    		void* features = nullptr;
    		// Core features
    		vk::PhysicalDeviceVulkan13Features features13{};
    		// Extension features
    		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    		void** nextFeature = &features;
    		if (data.properties.apiVersion >= vk::ApiVersion13)
    		{
    			*nextFeature = &features13;
    			nextFeature = &features13.pNext;
    		}

    		deviceFeatures.samplerAnisotropy = vk::True;
    		if (data.dynamicRendering == device_feature_type::core)
    		{
    			features13.dynamicRendering = vk::True;
    		}
    		else if (data.dynamicRendering == device_feature_type::extension)
    		{
    			extensions.push_back(vk::KHRDynamicRenderingExtensionName);
    			*nextFeature = &dynamicRenderingFeatures;
    			nextFeature = &dynamicRenderingFeatures.pNext;

    			dynamicRenderingFeatures.dynamicRendering = vk::True;
    		}

    		constexpr float queuePriorities[] = { 1.f, 1.f };
    		eastl::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
    		if (data.graphicsQueue.familyIndex == data.transferQueue.familyIndex)
    		{
    			const auto queueCount = static_cast<std::uint32_t>(data.graphicsQueue.queueIndex != data.transferQueue.queueIndex ? 2 : 1);
    			queueCreateInfo.push_back({ {}, data.graphicsQueue.familyIndex, queueCount, queuePriorities });
    		}
    		else
    		{
    			queueCreateInfo.push_back({ {}, data.graphicsQueue.familyIndex, 1, queuePriorities });
    			queueCreateInfo.push_back({ {}, data.transferQueue.familyIndex, 1, queuePriorities });
    		}

    		return data.device.createDevice({ {},
    			queueCreateInfo, {}, extensions, &deviceFeatures, features
    		});
    	}
	}

	render_manager_vulkan* render_manager_vulkan::s_instanceVulkan = nullptr;

	bool render_manager_vulkan::init(const create_info& info)
	{
    	s_instanceVulkan = this;

		if (!super::init(info))
		{
			return false;
		}

		if (!create_instance(info))
		{
			log::fatal("[render_manager_vulkan::init] Failed to create Vulkan instance!");
			return false;
		}
    	if (!window_manager_vulkan::instance()->on_instance_created(m_ctx))
    	{
    		log::fatal("[render_manager_vulkan::init] Failed to create Vulkan surfaces!");
    		return false;
    	}

    	if (!create_device())
    	{
    		log::fatal("[render_manager_vulkan::init] Failed to create Vulkan device!");
    		return false;
    	}

		return true;
	}

	void render_manager_vulkan::clear()
	{
		if (m_ctx.m_instance != nullptr)
		{
			window_manager_vulkan::instance()->clear_vulkan();

			m_ctx.m_device.destroy();
			m_ctx.m_instance.clear();
		}

    	s_instanceVulkan = nullptr;
		super::clear();
	}

	bool render_manager_vulkan::create_instance(const create_info& info)
	{
		auto instance = vulkan::instance_builder()
			.set_application_name(info.appName)
			.set_engine_name("RatsEngine")
			.set_application_version(0, 1, 0)
			.set_engine_version(0, 1, 0)
			.add_required_extensions(window_manager_vulkan::instance()->required_instance_extensions())
			.add_required_extensions(RequiredInstanceExtensions)
			.set_max_vulkan_version(vk::ApiVersion13)
			.build();
		if (instance == nullptr)
		{
			return false;
		}
		m_ctx.m_instance = std::move(instance);
		return true;
	}

	bool render_manager_vulkan::create_device()
    {
    	const auto mainSurface = window_manager_vulkan::instance()->surface(engine::window_manager::instance()->main_window_id());

    	auto availableDevicesView = m_ctx.i().enumeratePhysicalDevices().value | std::ranges::views::transform([&](const vk::PhysicalDevice& device) {
    		return calculate_physical_device_score(device, mainSurface);
    	}) | std::ranges::views::filter([](const physical_device_data& data) {
    		return data.score > 0;
    	});
    	if (availableDevicesView.empty())
    	{
    		log::fatal("[render_manager_vulkan::create_device] Couldn't find any supported physical devices!");
    		return false;
    	}

    	std::vector availableDevices(availableDevicesView.begin(), availableDevicesView.end());
    	std::ranges::max_element(availableDevices, std::greater(), [](const physical_device_data& data) {
			return data.VRAM;
		})->score += 50;
		std::ranges::sort(availableDevices, std::greater(), [](const physical_device_data& data) {
			return data.score;
		});
    	log::log("[render_manager_vulkan::create_device] Found {} devices:", availableDevices.size());
    	for (std::size_t index = 0; index < availableDevices.size(); ++index)
    	{
    		log::log("[render_manager_vulkan::create_device]   {}. {}",
    			index + 1, availableDevices[index].properties.deviceName.data());
    	}

    	const auto deviceResult = engine::create_device(availableDevices.front());
    	if (deviceResult.result != vk::Result::eSuccess)
    	{
    		log::fatal("[render_manager_vulkan::create_device] Failed to create Vulkan device! Error: {}", deviceResult.result);
    		return false;
    	}

    	m_ctx.m_device = deviceResult.value;
		return true;
	}

	window_manager_vulkan* window_manager_vulkan::s_instanceVulkan = nullptr;

	void window_manager_vulkan::clear_vulkan()
	{
    	if (!m_windowDataVulkan.empty())
    	{
    		const auto& ctx = render_manager_vulkan::instance()->ctx();
    		for (const auto& [id, data] : m_windowDataVulkan)
    		{
    			ctx.i().destroySurfaceKHR(data.surface);
    		}
    		m_windowDataVulkan.clear();
    	}
	}

	bool window_manager_vulkan::on_instance_created(const vulkan_context& ctx)
    {
    	const auto windowManager = engine::window_manager::instance();
    	return std::ranges::all_of(windowManager->window_ids(), [this, &ctx](const window_id& id) {
    		const auto surface = create_surface(ctx, id);
    		if (surface == nullptr)
    		{
				log::error("[window_manager_vulkan::on_instance_created] Failed to create surface for window {}", id);
				return false;
			}
			m_windowDataVulkan.emplace(id, window_data_vulkan{ surface });
    		return true;
    	});
    }

	vk::SurfaceKHR window_manager_vulkan::surface(const window_id& id) const
	{
    	const auto iter = m_windowDataVulkan.find(id);
    	return iter != m_windowDataVulkan.end() ? iter->second.surface : nullptr;
	}

	void window_manager_vulkan::on_init()
	{
		s_instanceVulkan = this;
	}
	void window_manager_vulkan::on_clear()
	{
		s_instanceVulkan = nullptr;
	}

	void window_manager_vulkan::on_destroy_window(const window_id& id)
    {
    	const auto iter = m_windowDataVulkan.find(id);
    	if (iter != m_windowDataVulkan.end())
    	{
    		render_manager_vulkan::instance()->ctx().i().destroySurfaceKHR(iter->second.surface);
    		m_windowDataVulkan.erase(iter);
    	}
    }
}