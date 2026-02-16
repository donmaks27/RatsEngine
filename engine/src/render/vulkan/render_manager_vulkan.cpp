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

    	struct physical_device_score
		{
			vk::PhysicalDevice device;
			vk::PhysicalDeviceProperties properties;

			std::uint32_t score = 0;
		};
    	physical_device_score calculate_physical_device_score(const vk::PhysicalDevice& device, const vk::SurfaceKHR& mainSurface)
		{
    		physical_device_score result{ .device = device };

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
    		}
    		else
    		{
    			requiredExtensions.push_back(vk::KHRDynamicRenderingExtensionName);
    			*nextFeature = &dynamicRenderingFeatures;
    			nextFeature = &dynamicRenderingFeatures.pNext;
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
			const auto queueFamilies = device.getQueueFamilyProperties2();
			const bool hasGraphicsQueueFamily = std::ranges::any_of(queueFamilies, [](const vk::QueueFamilyProperties2& queueFamily) {
				return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
			});
			if (!hasGraphicsQueueFamily)
			{
				return result;
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

    		result.score = 1;
			return result;
		}
	}

	bool vulkan::render_manager::init(const create_info& info)
	{
		if (!super::init(info))
		{
			return false;
		}
		m_windowManagerVulkan = dynamic_cast<window_manager*>(engine::window_manager::instance());
    	m_windowManagerVulkan->m_renderManagerVulkan = this;

		if (!create_instance(info))
		{
			log::fatal("[vulkan::render_manager::init] Failed to create Vulkan instance!");
			return false;
		}
    	if (!m_windowManagerVulkan->on_instance_created(m_apiCtx))
    	{
    		log::fatal("[vulkan::render_manager::init] Failed to create Vulkan surfaces!");
    		return false;
    	}

    	if (!create_device())
    	{
    		log::fatal("[vulkan::render_manager::init] Failed to create Vulkan device!");
    		return false;
    	}

		return true;
	}

	void vulkan::render_manager::clear()
	{
		if (m_apiCtx.m_instance != nullptr)
		{
			m_windowManagerVulkan->clear_vulkan();
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

	bool vulkan::render_manager::create_instance(const create_info& info)
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
				log::fatal("[vulkan::render_manager::init] Some of the validation layers not supported on this device!");
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
			log::fatal("[vulkan::render_manager::init] Failed to create Vulkan instance! Error: {}", instance.result);
			return false;
		}
		m_apiCtx.m_instance = instance.value;
    	vk::detail::defaultDispatchLoaderDynamic.init(m_apiCtx.i(), vkGetInstanceProcAddr);

		if constexpr (config::vulkan::validation_layers)
		{
			const auto debugMessenger = m_apiCtx.i().createDebugUtilsMessengerEXT(debugMessengerInfo);
			if (debugMessenger.result != vk::Result::eSuccess)
			{
				log::fatal("[vulkan::render_manager::init] Failed to create Vulkan debug messenger! Error: {}", debugMessenger.result);
				return false;
			}
			m_debugMessenger = debugMessenger.value;
		}
    	return true;
	}

	bool vulkan::render_manager::create_device()
    {
    	const auto mainSurface = m_windowManagerVulkan->surface(engine::window_manager::instance()->main_window_id());

    	auto availableDevicesView = m_apiCtx.i().enumeratePhysicalDevices().value | std::ranges::views::transform([&](const vk::PhysicalDevice& device) {
    		return calculate_physical_device_score(device, mainSurface);
    	}) | std::ranges::views::filter([](const physical_device_score& data) {
    		return data.score > 0;
    	});
    	if (availableDevicesView.empty())
    	{
    		log::fatal("[engine::vulkan::render_manager::create_device] Couldn't find any supported physical devices!");
    		return false;
    	}

		std::vector devices(availableDevicesView.begin(), availableDevicesView.end());
		std::ranges::sort(devices, std::greater<>(), [](const physical_device_score& data) {
			return data.score;
		});
    	log::log("[engine::vulkan::render_manager::create_device] Found {} devices:", devices.size());
    	for (std::size_t index = 0; index < devices.size(); ++index)
    	{
    		log::log("[engine::vulkan::render_manager::create_device]   {}. {}",
    			index + 1, devices[index].properties.deviceName.data());
    	}

		return true;
	}

	void vulkan::window_manager::clear_vulkan()
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

	bool vulkan::window_manager::on_instance_created(const api_context& ctx)
    {
    	const auto windowManager = engine::window_manager::instance();
    	return std::ranges::all_of(windowManager->window_ids(), [this, &ctx](const window_id& id) {
    		const auto surface = create_surface(ctx, id);
    		if (surface == nullptr)
    		{
				log::error("[vulkan::window_manager::on_instance_created] Failed to create surface for window {}", id);
				return false;
			}
			m_windowDataVulkan.emplace(id, window_data_vulkan{ surface });
    		return true;
    	});
    }

	vk::SurfaceKHR vulkan::window_manager::surface(const window_id& id) const
	{
    	const auto iter = m_windowDataVulkan.find(id);
    	return iter != m_windowDataVulkan.end() ? iter->second.surface : nullptr;
	}

	void vulkan::window_manager::on_destroy_window(const window_id& id)
    {
    	const auto iter = m_windowDataVulkan.find(id);
    	if (iter != m_windowDataVulkan.end())
    	{
    		api_ctx().i().destroySurfaceKHR(iter->second.surface);
    		m_windowDataVulkan.erase(iter);
    	}
    }
}