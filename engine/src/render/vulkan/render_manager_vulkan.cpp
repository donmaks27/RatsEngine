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

    	if (!pick_physical_device())
    	{
    		log::fatal("[vulkan::render_manager::init] Failed to pick Vulkan physical device!");
    		return false;
    	}

		return true;
	}

	void vulkan::render_manager::clear()
	{
		if (m_apiCtx.m_instance != nullptr)
		{
			m_windowManagerVulkan->clear_vulkan();
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

	bool vulkan::render_manager::create_instance(const create_info& info)
	{
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

	bool vulkan::render_manager::pick_physical_device()
	{
    	const auto devices = m_apiCtx.i().enumeratePhysicalDevices();
    	if (devices.result != vk::Result::eSuccess)
    	{
    		log::fatal("[vulkan::render_manager::pick_physical_device] Failed to enumerate Vulkan physical devices! Error: {}", devices.result);
    		return false;
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