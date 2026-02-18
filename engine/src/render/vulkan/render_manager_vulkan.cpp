#include <engine/render/vulkan/render_manager_vulkan.h>

#include <engine/render/window_manager.h>

#include <EASTL/span.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace engine
{
    namespace
	{
    	constexpr eastl::array RequiredInstanceExtensions = { vk::KHRGetSurfaceCapabilities2ExtensionName };
    	constexpr eastl::array RequiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
    	constexpr auto MinVulkanApiVersion = vk::ApiVersion11;

		vk::Bool32 vulkan_debug_callback(const vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
			const vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* data, void* userData)
		{
			switch (severity)
			{
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
				log::warning("[Vulkan] {}", data->pMessage);
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
				log::error("[Vulkan] {}", data->pMessage);
				break;
			default: ;
			}
			return vk::False;
		}

		[[nodiscard]] bool check_validation_layer_support(const eastl::span<const char*> validationLayers)
		{
			if constexpr (config::vulkan::validation_layers)
			{
				const auto availableLayers = vk::enumerateInstanceLayerProperties().value;
				return std::ranges::all_of(validationLayers, [&availableLayers](const char* layer) {
					return std::ranges::any_of(availableLayers, [&layer](const vk::LayerProperties& availableLayer) {
						return strcmp(availableLayer.layerName, layer) == 0;
					});
				});
			}
			else
			{
				return true;
			}
		}

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
				return (queueFamily.queueFamilyProperties.queueCount >= 2) &&
					!!(queueFamily.queueFamilyProperties.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer));
			});
    		if (graphicsIter != result.queueProperties.end())
    		{
    			result.graphicsQueue.familyIndex = static_cast<std::uint32_t>(std::distance(result.queueProperties.begin(), graphicsIter));
    			result.graphicsQueue.queueIndex = 0;
    			result.transferQueue.familyIndex = result.graphicsQueue.familyIndex;
    			result.transferQueue.queueIndex = 1;
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
    			queueCreateInfo.push_back({ {}, data.graphicsQueue.familyIndex, 2, queuePriorities });
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

	bool render_manager_vulkan::init(const create_info& info)
	{
		if (!super::init(info))
		{
			return false;
		}
		m_windowManagerVulkan = dynamic_cast<window_manager_vulkan*>(engine::window_manager::instance());
    	m_windowManagerVulkan->m_renderManagerVulkan = this;

		if (!create_instance(info))
		{
			log::fatal("[render_manager_vulkan::init] Failed to create Vulkan instance!");
			return false;
		}
    	if (!m_windowManagerVulkan->on_instance_created(m_apiCtx))
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
		if (m_apiCtx.m_instance != nullptr)
		{
			m_windowManagerVulkan->clear_vulkan();

			m_apiCtx.m_device.destroy();
			if constexpr (config::vulkan::validation_layers)
			{
				if (m_debugMessenger != nullptr)
				{
					m_apiCtx.i().destroyDebugUtilsMessengerEXT(m_debugMessenger);
					m_debugMessenger = nullptr;
				}
			}
			m_apiCtx.m_instance.destroy();
		}
		m_apiCtx = {};

		super::clear();
	}

	bool render_manager_vulkan::create_instance(const create_info& info)
	{
    	vk::detail::defaultDispatchLoaderDynamic.init(vkGetInstanceProcAddr);

		eastl::vector<const char*> validationLayers;
		vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		auto instanceExtensions = m_windowManagerVulkan->required_instance_extensions();
    	instanceExtensions.insert(instanceExtensions.end(), RequiredInstanceExtensions.begin(), RequiredInstanceExtensions.end());
		if constexpr (config::vulkan::validation_layers)
		{
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
			if (!check_validation_layer_support(validationLayers))
			{
				log::fatal("[render_manager_vulkan::init] Some of the validation layers not supported on this device!");
				return false;
			}
			instanceExtensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

    	constexpr auto maxInstanceVersion = vk::ApiVersion13;
    	const auto supportedInstanceVersion = vk::enumerateInstanceVersion().value;
		vk::ApplicationInfo appInfo{};
		appInfo.pApplicationName = info.appName.c_str();
		appInfo.applicationVersion = vk::makeApiVersion(0, 0, 1, 0);
		appInfo.pEngineName = "RatsEngine";
		appInfo.engineVersion = vk::makeApiVersion(0, 0, 1, 0);
		appInfo.apiVersion = std::min(maxInstanceVersion, supportedInstanceVersion);
		vk::InstanceCreateInfo instanceInfo{
			{}, &appInfo, validationLayers, instanceExtensions
		};
		if constexpr (config::vulkan::validation_layers)
		{
			debugMessengerInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
			debugMessengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
				| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
			debugMessengerInfo.pfnUserCallback = vulkan_debug_callback;
			instanceInfo.pNext = &debugMessengerInfo;
		}
		const auto instance = vk::createInstance(instanceInfo);
		if (instance.result != vk::Result::eSuccess)
		{
			log::fatal("[render_manager_vulkan::init] Failed to create Vulkan instance! Error: {}", instance.result);
			return false;
		}
		m_apiCtx.m_instance = instance.value;
    	vk::detail::defaultDispatchLoaderDynamic.init(m_apiCtx.i(), vkGetInstanceProcAddr);

		if constexpr (config::vulkan::validation_layers)
		{
			const auto debugMessenger = m_apiCtx.i().createDebugUtilsMessengerEXT(debugMessengerInfo);
			if (debugMessenger.result != vk::Result::eSuccess)
			{
				log::fatal("[render_manager_vulkan::init] Failed to create Vulkan debug messenger! Error: {}", debugMessenger.result);
				return false;
			}
			m_debugMessenger = debugMessenger.value;
		}
    	return true;
	}

	bool render_manager_vulkan::create_device()
    {
    	const auto mainSurface = m_windowManagerVulkan->surface(engine::window_manager::instance()->main_window_id());

    	auto availableDevicesView = m_apiCtx.i().enumeratePhysicalDevices().value | std::ranges::views::transform([&](const vk::PhysicalDevice& device) {
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

    	const auto deviceResult = engine::create_device(availableDevices[0]);
    	if (deviceResult.result != vk::Result::eSuccess)
    	{
    		log::fatal("[render_manager_vulkan::create_device] Failed to create Vulkan device! Error: {}", deviceResult.result);
    		return false;
    	}

    	m_apiCtx.m_device = deviceResult.value;
		return true;
	}

	void window_manager_vulkan::clear_vulkan()
	{
    	if (!m_windowDataVulkan.empty())
    	{
    		const auto& ctx = api_ctx();
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

	void window_manager_vulkan::on_destroy_window(const window_id& id)
    {
    	const auto iter = m_windowDataVulkan.find(id);
    	if (iter != m_windowDataVulkan.end())
    	{
    		api_ctx().i().destroySurfaceKHR(iter->second.surface);
    		m_windowDataVulkan.erase(iter);
    	}
    }
}