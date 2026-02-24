#include <engine/render/vulkan/builder/device_builder.h>

namespace engine::vulkan
{
    device_builder& device_builder::add_required_extension(const char* extension)
    {
        m_requiredExtensions.emplace(extension);
        return *this;
    }
    device_builder& device_builder::add_required_extensions(eastl::span<const char* const> extensions)
    {
        m_requiredExtensions.reserve(m_requiredExtensions.size() + extensions.size());
        for (const auto& extension : extensions)
        {
            m_requiredExtensions.emplace(extension);
        }
        return *this;
    }

    device_builder& device_builder::set_min_vulkan_version(const std::uint32_t version)
    {
        m_minVulkanVersion = version;
        return *this;
    }
    device_builder& device_builder::feature_dynamic_rendering(const feature feature)
    {
        m_featureDynamicRendering = feature;
        return *this;
    }

    device_builder& device_builder::collect_physical_devices(const instance& i, const vk::SurfaceKHR& surface)
    {
        m_physicalDevices.clear();

        const auto physicalDevices = i->enumeratePhysicalDevices().value;
        auto availableDevices = physicalDevices | std::ranges::views::transform([this, &surface](const vk::PhysicalDevice& device) {
            return get_physical_device_data(device, surface);
        }) | std::ranges::views::filter([](const physical_device_data& data) {
            return data.score > 0;
        });
        if (availableDevices.empty())
        {
            return *this;
        }
        m_physicalDevices.reserve(physicalDevices.size());
        std::ranges::copy(availableDevices, std::back_inserter(m_physicalDevices));

        std::ranges::max_element(m_physicalDevices, std::greater(), [](const physical_device_data& data) {
            return data.VRAM;
        })->score += 50;
        std::ranges::sort(m_physicalDevices, std::greater(), [](const physical_device_data& data) {
            return data.score;
        });
        log::log("[vulkan::device_builder::collect_physical_devices] Found {} devices:", m_physicalDevices.size());
        for (std::size_t index = 0; index < m_physicalDevices.size(); ++index)
        {
            log::log("[vulkan::device_builder::collect_physical_devices]   {}. {}",
                index + 1, m_physicalDevices[index].name);
        }
        return *this;
    }
    device_builder& device_builder::get_physical_devices(eastl::vector<std::string_view>& devices)
    {
    	devices.clear();
    	devices.reserve(m_physicalDevices.size());
    	std::ranges::transform(m_physicalDevices, std::back_inserter(devices), [](const physical_device_data& data) {
    		return std::string_view(data.name);
    	});
    	return *this;
    }
	device_builder& device_builder::set_preferred_physical_device(std::string name)
    {
    	m_preferredDeviceName = std::move(name);
    	return *this;
    }

    device_builder::physical_device_data device_builder::get_physical_device_data(const vk::PhysicalDevice& device,
        const vk::SurfaceKHR& surface) const
    {
    	const auto properties = device.getProperties2().properties;
    	physical_device_data result{
    		.device = device, .name = properties.deviceName, .vulkanVersion = properties.apiVersion
    	};
    	if (properties.apiVersion < m_minVulkanVersion)
    	{
    		return result;
    	}

    	if (device.getSurfacePresentModesKHR(surface).value.empty() || device.getSurfaceFormats2KHR({ surface }).value.empty())
    	{
    		return result;
    	}
    	if (!get_physical_device_queues(result))
    	{
    		return result;
    	}
    	if (!get_physical_device_features(result))
    	{
    		return result;
    	}

   		const auto& memoryProperties = device.getMemoryProperties2().memoryProperties;
   		const auto heapIter = std::ranges::find_if(memoryProperties.memoryHeaps, [](const vk::MemoryHeap& heap) {
   			return !!(heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal);
   		});
   		if (heapIter == memoryProperties.memoryHeaps.end())
   		{
   			return result;
   		}
   		result.VRAM = heapIter->size;

   		switch (properties.deviceType)
   		{
		   case vk::PhysicalDeviceType::eDiscreteGpu:   result.score += 1000; break;
		   case vk::PhysicalDeviceType::eIntegratedGpu: result.score += 500;  break;
		   default: ;
   		}
    	const bool hasOptionalFeatures = (m_featureDynamicRendering == feature::optional) && (result.featureDynamicRendering != location::none);
		if (hasOptionalFeatures)
		{
			result.score += 20;
		}
    	return result;
    }
    bool device_builder::get_physical_device_queues(physical_device_data& data)
    {
    	const auto queueProperties = data.device.getQueueFamilyProperties2();
    	auto graphicsIter = std::ranges::find_if(queueProperties, [](const vk::QueueFamilyProperties2& queueFamily) {
			return !!(queueFamily.queueFamilyProperties.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer));
		});
    	if (graphicsIter != queueProperties.end())
    	{
    		data.graphicsQueue.familyIndex = static_cast<std::uint32_t>(std::distance(queueProperties.begin(), graphicsIter));
    		data.graphicsQueue.queueIndex = 0;
    		data.transferQueue.familyIndex = data.graphicsQueue.familyIndex;
    		const auto queueCount = queueProperties[data.graphicsQueue.familyIndex].queueFamilyProperties.queueCount;
    		data.transferQueue.queueIndex = queueCount >= 2 ? 1 : 0;
    		return true;
    	}

    	graphicsIter = std::ranges::find_if(queueProperties, [](const vk::QueueFamilyProperties2& queueFamily) {
			return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
		});
    	const auto transferIter = std::ranges::find_if(graphicsIter + 1, queueProperties.end(), [](const vk::QueueFamilyProperties2& queueFamily) {
			return !!(queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer);
		});
    	if ((graphicsIter == queueProperties.end()) || (transferIter == queueProperties.end()))
    	{
    		return false;
    	}

    	data.graphicsQueue.familyIndex = static_cast<std::uint32_t>(std::distance(queueProperties.begin(), graphicsIter));
    	data.graphicsQueue.queueIndex = 0;
    	data.transferQueue.familyIndex = static_cast<std::uint32_t>(std::distance(queueProperties.begin(), transferIter));
    	data.transferQueue.queueIndex = 0;
    	return true;
    }
    bool device_builder::get_physical_device_features(physical_device_data& data) const
    {
    	eastl::vector requiredExtensions = m_requiredExtensions;
    	eastl::vector_map<const char*, bool> optionalExtensions;
    	vk::PhysicalDeviceFeatures2 features;
   		// Core features
   		vk::PhysicalDeviceVulkan13Features features13;
   		// Extension features
   		vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures;
   		void** nextFeature = &features.pNext;
    	if (data.vulkanVersion >= vk::ApiVersion13)
    	{
    		*nextFeature = &features13;
    		nextFeature = &features13.pNext;
    	}

    	if ((m_featureDynamicRendering != feature::none) && (data.vulkanVersion < vk::ApiVersion13))
    	{
    		*nextFeature = &dynamicRenderingFeatures;
    		nextFeature = &dynamicRenderingFeatures.pNext;
    		if (m_featureDynamicRendering == feature::required)
    		{
    			requiredExtensions.push_back(vk::KHRDynamicRenderingExtensionName);
    		}
    		else
    		{
    			optionalExtensions[vk::KHRDynamicRenderingExtensionName] = false;
    		}
    	}

    	// Check device extensions
    	const auto deviceExtensions = data.device.enumerateDeviceExtensionProperties().value;
    	const bool hasRequiredExtensions = std::ranges::all_of(requiredExtensions, [&deviceExtensions](const char* requiredExtension) {
			return std::ranges::any_of(deviceExtensions, [&requiredExtension](const vk::ExtensionProperties& extension) {
				return std::strcmp(requiredExtension, extension.extensionName) == 0;
			});
		});
    	if (!hasRequiredExtensions)
    	{
    		return false;
    	}
    	std::ranges::for_each(optionalExtensions, [&deviceExtensions](eastl::pair<const char*, bool>& data) {
			data.second = std::ranges::any_of(deviceExtensions, [&data](const vk::ExtensionProperties& extension) {
				return std::strcmp(data.first, extension.extensionName) == 0;
			});
		});

    	// Check device features
    	data.device.getFeatures2(&features);
    	if (!features.features.samplerAnisotropy)
    	{
    		return false;
    	}
    	if (m_featureDynamicRendering != feature::none)
    	{
    		const bool coreFeature = data.vulkanVersion >= vk::ApiVersion13;
    		const bool featureSupported = coreFeature ? features13.dynamicRendering : dynamicRenderingFeatures.dynamicRendering;
    		if ((m_featureDynamicRendering == feature::required) && !featureSupported)
    		{
    			return false;
    		}
    		if (featureSupported)
    		{
    			data.featureDynamicRendering = coreFeature ? location::core : location::extension;
    		}
    	}

    	data.extensions.reserve(requiredExtensions.size() + optionalExtensions.size());
    	data.extensions = requiredExtensions;
    	std::ranges::copy(optionalExtensions | std::ranges::views::filter([](const eastl::pair<const char*, bool>& data) {
			return data.second;
		}) | std::ranges::views::transform([](const eastl::pair<const char*, bool>& data) {
			return data.first;
		}), std::back_inserter(data.extensions));
    	return true;
    }

    device device_builder::build(const instance& i)
    {
        return {};
    }
}
