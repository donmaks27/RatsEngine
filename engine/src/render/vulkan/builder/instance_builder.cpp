#include <engine/render/vulkan/builder/instance_builder.h>

namespace engine::vulkan
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
	}

    instance_builder& instance_builder::set_application_name(std::string name)
    {
        m_applicationName = std::move(name);
        return *this;
    }
    instance_builder& instance_builder::set_engine_name(std::string name)
    {
        m_engineName = std::move(name);
        return *this;
    }

    instance_builder& instance_builder::set_application_version(const std::uint8_t major, const std::uint8_t minor, const std::uint8_t patch)
    {
        m_applicationVersion = vk::makeApiVersion(static_cast<std::uint8_t>(0), major, minor, patch);
        return *this;
    }
    instance_builder& instance_builder::set_engine_version(const std::uint8_t major, const std::uint8_t minor, const std::uint8_t patch)
    {
        m_engineVersion = vk::makeApiVersion(static_cast<std::uint8_t>(0), major, minor, patch);
        return *this;
    }

    instance_builder& instance_builder::add_required_extension(const char* extension)
    {
        m_requiredExtensions.emplace_back(extension);
        return *this;
    }
    instance_builder& instance_builder::add_required_extensions(eastl::span<const char* const> extensions)
    {
        m_requiredExtensions.reserve(m_requiredExtensions.size() + extensions.size());
        std::ranges::copy(extensions, std::back_inserter(m_requiredExtensions));
        return *this;
    }

    instance_builder& instance_builder::set_max_vulkan_version(const std::uint32_t version)
    {
        m_maxVulkanVersion = version;
        return *this;
    }

    instance instance_builder::build()
    {
        vk::detail::defaultDispatchLoaderDynamic.init(vkGetInstanceProcAddr);

		eastl::vector<const char*> validationLayers;
		vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
		if constexpr (config::vulkan::validation_layers)
		{
			validationLayers.emplace_back("VK_LAYER_KHRONOS_validation");
			if (!check_validation_layer_support(validationLayers))
			{
				log::fatal("[vulkan::instance_builder::build] Some of the validation layers are not supported on this device!");
				return {};
			}
			m_requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

    	const auto supportedInstanceVersion = vk::enumerateInstanceVersion().value;
		const vk::ApplicationInfo appInfo{
			m_applicationName.c_str(), m_applicationVersion,
			m_engineName.c_str(), m_engineVersion,
			std::min(m_maxVulkanVersion, supportedInstanceVersion)
		};
		vk::InstanceCreateInfo instanceInfo{
			{}, &appInfo, validationLayers, m_requiredExtensions
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
		auto vulkanInstance = vk::createInstanceUnique(instanceInfo);
		if (vulkanInstance.result != vk::Result::eSuccess)
		{
			log::fatal("[vulkan::instance_builder::build] Failed to create Vulkan instance! Error: {}", vulkanInstance.result);
			return {};
		}

		instance result;
		result.m_instance = std::move(vulkanInstance.value);
    	vk::detail::defaultDispatchLoaderDynamic.init(*result, vkGetInstanceProcAddr);
		if constexpr (config::vulkan::validation_layers)
		{
			auto debugMessenger = result->createDebugUtilsMessengerEXTUnique(debugMessengerInfo);
			if (debugMessenger.result != vk::Result::eSuccess)
			{
				log::fatal("[vulkan::instance_builder::build] Failed to create Vulkan debug messenger! Error: {}", debugMessenger.result);
				return {};
			}
			result.m_debugMessenger = std::move(debugMessenger.value);
		}
        return result;
    }
}
