#include <engine/render/vulkan/builder/device_builder.h>

namespace engine::vulkan
{
	namespace
	{
        using feature_location = device_builder::feature_location;

		struct device_features_data
        {
            explicit device_features_data(const std::uint32_t vulkanVersion)
				: vulkanVersion(vulkanVersion)
            {
                nextFeature = &data.pNext;
                if (vulkanVersion >= vk::ApiVersion13)
                {
                    *nextFeature = &features13;
                    nextFeature = &features13.pNext;
                }
            }

            vk::PhysicalDeviceFeatures2 data;
            // Core features
            vk::PhysicalDeviceVulkan13Features features13;
            // Extension features
            vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures;
            vk::PhysicalDeviceSynchronization2Features synchronization2Features;

            void** nextFeature;
            std::uint32_t vulkanVersion;


			[[nodiscard]] vk::PhysicalDeviceFeatures2* operator->() noexcept { return &data; }
            [[nodiscard]] vk::PhysicalDeviceFeatures2& operator*() noexcept { return data; }

            void request(const eastl::vector_map<device_feature, device_feature_type>& features,
                eastl::vector<const char*>& requiredExtensions, eastl::vector_map<const char*, bool>& optionalExtensions);
            [[nodiscard]] bool check(const eastl::vector_map<device_feature, device_feature_type>& features,
                eastl::vector_map<device_feature, feature_location>& locations) const;
            void require(const eastl::vector_map<device_feature, feature_location>& features);

		private:

            template<device_feature Name>
            void request(device_feature_type type, eastl::vector<const char*>& requiredExtensions, eastl::vector_map<const char*, bool>& optionalExtensions);
            template<device_feature Name>
            [[nodiscard]] bool check(device_feature_type type, eastl::vector_map<device_feature, feature_location>& locations) const;
            template<device_feature Name>
            void require(feature_location location);
		};

		template<device_feature Name>
        struct device_feature_info
        {
            static constexpr const char* extensionName = nullptr;
            [[nodiscard]] 
        	static constexpr bool core(const std::uint32_t version) { return true; }
            static void chain(device_features_data& data) {}
            [[nodiscard]]
			static feature_location location(const device_features_data& data, const std::uint32_t version) { return feature_location::none; }
            static void mark(device_features_data& data, feature_location location) {}
        };
        template<>
        struct device_feature_info<device_feature::sampler_anisotropy>
        {
            static constexpr const char* extensionName = nullptr;
            [[nodiscard]]
            static bool core(std::uint32_t) { return true; }
            static void chain(device_features_data&) {}
            [[nodiscard]]
            static feature_location location(const device_features_data& data, std::uint32_t)
            {
                return !data.data.features.samplerAnisotropy ? feature_location::none : feature_location::core;
            }
            static void mark(device_features_data& data, feature_location)
            {
				data.data.features.samplerAnisotropy = vk::True;
            }
        };
        template<>
        struct device_feature_info<device_feature::dynamic_render>
        {
            static constexpr const char* extensionName = vk::KHRDynamicRenderingExtensionName;
            [[nodiscard]]
            static bool core(const std::uint32_t version) { return version >= vk::ApiVersion13; }
            static void chain(device_features_data& data)
            {
                *data.nextFeature = &data.dynamicRenderingFeatures;
                data.nextFeature = &data.dynamicRenderingFeatures.pNext;
            }
            [[nodiscard]]
            static feature_location location(const device_features_data& data, const std::uint32_t version)
            {
                const bool coreFeature = core(version);
                const bool featureSupported = coreFeature ? data.features13.dynamicRendering : data.dynamicRenderingFeatures.dynamicRendering;
				return !featureSupported ? feature_location::none : (coreFeature ? feature_location::core : feature_location::extension);
            }
            static void mark(device_features_data& data, const feature_location location)
            {
                if (location == feature_location::core)
                {
                    data.features13.dynamicRendering = vk::True;
                }
                else if (location == feature_location::extension)
                {
                    data.dynamicRenderingFeatures.dynamicRendering = vk::True;
                }
            }
        };
        template<>
        struct device_feature_info<device_feature::synchronization2>
        {
            static constexpr const char* extensionName = vk::KHRSynchronization2ExtensionName;
            [[nodiscard]]
            static bool core(const std::uint32_t version) { return version >= vk::ApiVersion13; }
            static void chain(device_features_data& data)
            {
                *data.nextFeature = &data.synchronization2Features;
                data.nextFeature = &data.synchronization2Features.pNext;
            }
            [[nodiscard]]
            static feature_location location(const device_features_data& data, const std::uint32_t version)
            {
                const bool coreFeature = core(version);
                const bool featureSupported = coreFeature ? data.features13.synchronization2 : data.synchronization2Features.synchronization2;
                return !featureSupported ? feature_location::none : (coreFeature ? feature_location::core : feature_location::extension);
            }
            static void mark(device_features_data& data, const feature_location location)
            {
                if (location == feature_location::core)
                {
                    data.features13.synchronization2 = vk::True;
                }
                else if (location == feature_location::extension)
                {
                    data.synchronization2Features.synchronization2 = vk::True;
                }
            }
        };

        template <device_feature Name>
        void device_features_data::request(const device_feature_type type, eastl::vector<const char*>& requiredExtensions,
            eastl::vector_map<const char*, bool>& optionalExtensions)
        {
			using info = device_feature_info<Name>;
            if (!info::core(vulkanVersion))
            {
                info::chain(*this);
                if (info::extensionName != nullptr)
                {
                    if (type == device_feature_type::required)
                    {
                        requiredExtensions.push_back(info::extensionName);
                    }
                    else
                    {
                        optionalExtensions[info::extensionName] = false;
                    }
                }
            }
        }
        template <device_feature Name>
        bool device_features_data::check(const device_feature_type type, eastl::vector_map<device_feature, feature_location>& locations) const
        {
            using info = device_feature_info<Name>;
			const auto location = info::location(*this, vulkanVersion);
            if ((type == device_feature_type::required) && (location == feature_location::none))
            {
                return false;
			}
			locations[Name] = location;
            return true;
        }
        template <device_feature Name>
        void device_features_data::require(const feature_location location)
        {
            using info = device_feature_info<Name>;
            if (location != feature_location::core)
            {
	            info::chain(*this);
            }
			info::mark(*this, location);
        }

		void device_features_data::request(const eastl::vector_map<device_feature, device_feature_type>& features,
			eastl::vector<const char*>& requiredExtensions, eastl::vector_map<const char*, bool>& optionalExtensions)
		{
			for (const auto& [name, type] : features)
			{
				switch (name)
				{
                case device_feature::sampler_anisotropy: request<device_feature::sampler_anisotropy>(type, requiredExtensions, optionalExtensions); break;
                case device_feature::dynamic_render: request<device_feature::dynamic_render>(type, requiredExtensions, optionalExtensions); break;
                case device_feature::synchronization2: request<device_feature::synchronization2>(type, requiredExtensions, optionalExtensions); break;
				default:;
				}
			}
		}
		bool device_features_data::check(const eastl::vector_map<device_feature, device_feature_type>& features,
			eastl::vector_map<device_feature, feature_location>& locations) const
		{
            return std::ranges::all_of(features, [this, &locations](const eastl::pair<device_feature, device_feature_type>& pair) {
	            switch (pair.first)
	            {
                case device_feature::sampler_anisotropy: return check<device_feature::sampler_anisotropy>(pair.second, locations);
                case device_feature::dynamic_render: return check<device_feature::dynamic_render>(pair.second, locations);
                case device_feature::synchronization2: return check<device_feature::synchronization2>(pair.second, locations);
                default:;
	            }
                return false;
			});
		}
		void device_features_data::require(const eastl::vector_map<device_feature, device_builder::feature_location>& features)
		{
			for (const auto& [name, location] : features)
			{
				switch (name)
				{
				case device_feature::sampler_anisotropy: require<device_feature::sampler_anisotropy>(location); break;
				case device_feature::dynamic_render: require<device_feature::dynamic_render>(location); break;
				case device_feature::synchronization2: require<device_feature::synchronization2>(location); break;
				default:;
				}
			}
		}
	}

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
	device_builder& device_builder::prefer_dedicated_transfer_queue(const bool prefer)
    {
        m_preferDedicatedTransferQueue = prefer;
		return *this;
    }
    device_builder& device_builder::prefer_dedicated_compute_queue(const bool prefer)
    {
		m_preferDedicatedComputeQueue = prefer;
        return *this;
	}
    device_builder& device_builder::feature(const device_feature name, const device_feature_type type)
    {
        if (type == device_feature_type::none)
        {
            m_features.erase(name);
        }
        else
        {
			m_features[name] = type;
        }
        return *this;
    }

    device_builder& device_builder::collect_physical_devices(const instance& i, const vk::SurfaceKHR& surface)
    {
    	if (gather_physical_devices(i, surface))
    	{
    		calculate_physical_devices_score();
    		log::log("[vulkan::device_builder::collect_physical_devices] Found {} devices:", m_physicalDevices.size());
    		for (std::size_t index = 0; index < m_physicalDevices.size(); ++index)
    		{
    			log::log("[vulkan::device_builder::collect_physical_devices]   {}. {} ({})",
					index + 1, m_physicalDevices[index].name, m_physicalDevices[index].score);
    		}
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

	bool device_builder::gather_physical_devices(const instance& i, const vk::SurfaceKHR& surface)
	{
    	const auto physicalDevices = i->enumeratePhysicalDevices().value;
    	m_physicalDevices.clear();
    	m_physicalDevices.reserve(physicalDevices.size());
    	std::ranges::transform(physicalDevices, std::back_inserter(m_physicalDevices), [this, &surface](const vk::PhysicalDevice& device) {
			physical_device_data result;
			get_physical_device_data(result, device, surface);
			return result;
		});
    	eastl::erase_if(m_physicalDevices, [](const physical_device_data& data) {
			return data.device == nullptr;
		});
    	return !m_physicalDevices.empty();
	}
    void device_builder::get_physical_device_data(physical_device_data& outData, const vk::PhysicalDevice& device,
        const vk::SurfaceKHR& surface) const
    {
    	const auto properties = device.getProperties2().properties;
    	if (properties.apiVersion < m_minVulkanVersion)
    	{
    		return;
    	}

        const auto& memoryProperties = device.getMemoryProperties2().memoryProperties;
        const auto heapIter = std::ranges::find_if(memoryProperties.memoryHeaps, [](const vk::MemoryHeap& heap) {
            return !!(heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal);
        });
        if (heapIter == memoryProperties.memoryHeaps.end())
        {
            return;
        }
    	if (device.getSurfacePresentModesKHR(surface).value.empty() || device.getSurfaceFormats2KHR({ surface }).value.empty())
    	{
    		return;
    	}

        physical_device_data result = {
    		.device = device, 
    		.name = properties.deviceName, 
    		.vulkanVersion = properties.apiVersion, 
    		.deviceType = properties.deviceType, 
    		.VRAM = heapIter->size
    	};
    	if (!get_physical_device_features(result))
    	{
    		return;
    	}
    	get_physical_device_queues(result, surface);

    	outData = std::move(result);
    }
    void device_builder::get_physical_device_queues(physical_device_data& data, const vk::SurfaceKHR& surface) const
    {
        struct queue_info
        {
			std::uint32_t familyIndex = 0;
			bool graphics = false;
			bool present = false;
			bool compute = false;
            bool transfer = false;
        };
    	const auto queueProperties = data.device.getQueueFamilyProperties2();
        eastl::vector<queue_info> queueIndices(queueProperties.size());
        for (std::uint32_t i = 0; i < queueProperties.size(); i++)
        {
            queueIndices[i].familyIndex = i;
		}
        std::ranges::for_each(queueIndices, [&](queue_info& info) {
            const auto queueFlags = queueProperties[info.familyIndex].queueFamilyProperties.queueFlags;
			info.graphics = static_cast<bool>(queueFlags & vk::QueueFlagBits::eGraphics);
            info.present = data.device.getSurfaceSupportKHR(info.familyIndex, surface).value == vk::True;
			info.compute = static_cast<bool>(queueFlags & vk::QueueFlagBits::eCompute);
			info.transfer = static_cast<bool>(queueFlags & vk::QueueFlagBits::eTransfer);
        });
        std::ranges::sort(queueIndices, [](const queue_info& info1, const queue_info& info2) {
            if (info1.graphics != info2.graphics)
            {
                return info1.graphics;
            }
			const bool graphicsPresent1 = info1.graphics && info1.present;
			const bool graphicsPresent2 = info2.graphics && info2.present;
            if (graphicsPresent1 != graphicsPresent2)
            {
                return graphicsPresent1;
            }
            if (info1.compute != info2.compute)
            {
                return info1.compute;
            }
            if (info1.transfer != info2.transfer)
            {
                return info1.transfer;
            }
            return false;
        });

        const std::uint32_t graphicsIndex = queueIndices[0].familyIndex;
        const std::uint32_t presentIndex = std::ranges::find_if(queueIndices, [](const queue_info& info) {
            return info.present;
        })->familyIndex;
        std::uint32_t computeIndex = graphicsIndex;
        if (m_preferDedicatedComputeQueue)
        {
            const auto iter = std::ranges::find_if(queueIndices, [&](const queue_info& info) {
                return !info.graphics && info.compute;
            });
            if (iter != queueIndices.end())
            {
                computeIndex = iter->familyIndex;
            }
		}
        std::uint32_t transferIndex = graphicsIndex;
        if (m_preferDedicatedTransferQueue)
        {
            const auto iter = std::ranges::find_if(queueIndices, [&](const queue_info& info) {
                return !info.graphics && !info.compute && info.transfer;
            });
            if (iter != queueIndices.end())
            {
                transferIndex = iter->familyIndex;
            }
        }

        data.queues = {
			{ queue_type::graphics, { .familyIndex = graphicsIndex } },
			{ queue_type::present,  { .familyIndex = presentIndex  } },
            { queue_type::compute,  { .familyIndex = computeIndex  } },
			{ queue_type::transfer, { .familyIndex = transferIndex } },
        };
    }
    bool device_builder::get_physical_device_features(physical_device_data& data) const
    {
    	eastl::vector requiredExtensions = m_requiredExtensions;
    	eastl::vector_map<const char*, bool> optionalExtensions;
        device_features_data features(data.vulkanVersion);
        features.request(m_features, requiredExtensions, optionalExtensions);

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
    	data.device.getFeatures2(&*features);
        if (!features.check(m_features, data.features))
        {
            return false;
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

    void device_builder::calculate_physical_devices_score()
    {
    	std::ranges::for_each(m_physicalDevices, [this](physical_device_data& data) {
			switch (data.deviceType)
			{
			case vk::PhysicalDeviceType::eDiscreteGpu:   data.score += 1000; break;
			case vk::PhysicalDeviceType::eIntegratedGpu: data.score += 500;  break;
			default:;
			}
            const bool hasOptionalFeatures = std::ranges::any_of(m_features, [&data](const eastl::pair<device_feature, device_feature_type>& feature) {
				const auto iter = (feature.second == device_feature_type::optional) ? data.features.find(feature.first) : data.features.end();
				return (iter != data.features.end()) && (iter->second != feature_location::none);
            });
			if (hasOptionalFeatures)
			{
				data.score += 20;
			}
		});
    	std::ranges::max_element(m_physicalDevices, std::greater(), [](const physical_device_data& data) {
			return data.VRAM;
		})->score += 50;
    	std::ranges::sort(m_physicalDevices, std::greater(), [](const physical_device_data& data) {
			return data.score;
		});
    }

    device device_builder::build(const instance& i)
    {
        if (m_physicalDevices.empty())
        {
            log::fatal("[vulkan::device_builder::build] No allowed physical devices found!");
            return nullptr;
        }

        const auto deviceIter = !m_preferredDeviceName.empty() ? std::ranges::find_if(m_physicalDevices, [this](const physical_device_data& data) {
            return data.name == m_preferredDeviceName;
		}) : m_physicalDevices.end();
		const auto& physicalDevice = deviceIter != m_physicalDevices.end() ? *deviceIter : m_physicalDevices.front();

		device_features_data features(physicalDevice.vulkanVersion);
        features.require(physicalDevice.features);

        static constexpr float queuePriorities[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        eastl::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
        queueCreateInfo.reserve(4);
        for (const auto& [type, queueData] : physicalDevice.queues)
        {
            const auto infoIter = std::ranges::find(queueCreateInfo, queueData.familyIndex, [](const vk::DeviceQueueCreateInfo& info) {
                return info.queueFamilyIndex;
			});
            if (infoIter != queueCreateInfo.end())
            {
                infoIter->queueCount++;
            }
            else
            {
                queueCreateInfo.push_back({ {}, queueData.familyIndex, 1, queuePriorities });
			}
		}
        auto deviceValue = physicalDevice.device.createDeviceUnique({ {},
            queueCreateInfo, {}, physicalDevice.extensions, &features->features, features->pNext
		});
        if (deviceValue.result != vk::Result::eSuccess)
        {
            log::fatal("[vulkan::device_builder::build] Failed to create Vulkan device! Error: {}", deviceValue.result);
            return nullptr;
		}

        vk::detail::defaultDispatchLoaderDynamic.init(*i, deviceValue.value.get());

    	eastl::vector_map<queue_type, vulkan::queue> queues;
    	for (const auto& [type, queueData] : physicalDevice.queues)
    	{
    		auto& queue = queues[type];
    		queue.m_value = deviceValue.value->getQueue2({ {}, queueData.familyIndex, queueData.queueIndex });
    		queue.m_familyIndex = queueData.familyIndex;
    		queue.m_queueIndex = queueData.queueIndex;
    	}
        
        device result;
		result.m_value = std::move(deviceValue.value);
		result.m_physicalDevice = physicalDevice.device;
        for (const auto& [feature, location] : physicalDevice.features)
        {
            if (location != feature_location::none)
            {
                result.m_features.emplace(feature);
            }
		}
        result.m_queues = std::move(queues);
        return result;
    }
}
