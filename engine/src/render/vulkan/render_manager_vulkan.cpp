#include <engine/render/vulkan/render_manager_vulkan.h>

#include <engine/render/window_manager.h>

#include <EASTL/span.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace engine
{
    namespace
	{
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
			const auto availableLayers = vk::enumerateInstanceLayerProperties().value;
			return std::ranges::all_of(validationLayers, [&availableLayers](const char* layer) {
				return std::ranges::any_of(availableLayers, [&layer](const vk::LayerProperties& availableLayer) {
					return strcmp(availableLayer.layerName, layer) == 0;
				});
			});
		}
	}

	bool vulkan::render_manager::init(const create_info& info)
	{
		if (!super::init(info))
		{
			return false;
		}
		m_windowManagerVulkan = dynamic_cast<window_manager*>(engine::window_manager::instance());

    	vk::detail::defaultDispatchLoaderDynamic.init(vkGetInstanceProcAddr);

		eastl::vector<const char*> validationLayers;
		vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		auto instanceExtensions = m_windowManagerVulkan->get_required_extensions();
		if constexpr (config::debug)
		{
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
			if (!check_validation_layer_support(validationLayers))
			{
				log::fatal("[vulkan::render_manager::init] Some of the validation layers not supported on this device!");
				return false;
			}
			instanceExtensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		vk::ApplicationInfo appInfo{};
		appInfo.pApplicationName = info.appName.c_str();
		appInfo.applicationVersion = vk::makeApiVersion(0, 0, 1, 0);
		appInfo.pEngineName = "RatsEngine";
		appInfo.engineVersion = vk::makeApiVersion(0, 0, 1, 0);
		appInfo.apiVersion = vk::ApiVersion14;
		vk::InstanceCreateInfo instanceInfo{
			{}, &appInfo, validationLayers, instanceExtensions
		};
		if constexpr (config::debug)
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

		if constexpr (config::debug)
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

	void vulkan::render_manager::clear()
	{
		if (m_apiCtx.m_instance != nullptr)
		{
			if constexpr (config::debug)
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
}