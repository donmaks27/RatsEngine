#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <EASTL/span.h>
#include <EASTL/vector_set.h>

namespace engine::vulkan
{
    class device_builder final
    {
    public:
        enum class feature : std::uint8_t { none, optional, required };
        enum class location : std::uint8_t { none, extension, core };

        device_builder() = default;
        device_builder(const device_builder&) = delete;
        device_builder(device_builder&&) = delete;
        ~device_builder() = default;

        device_builder& operator=(const device_builder&) = delete;
        device_builder& operator=(device_builder&&) = delete;

        device_builder& add_required_extension(const char* extension);
        device_builder& add_required_extensions(eastl::span<const char* const> extensions);

        device_builder& set_min_vulkan_version(std::uint32_t version);
		device_builder& prefer_dedicated_transfer_queue(bool prefer = true);
		device_builder& prefer_dedicated_compute_queue(bool prefer = true);
        device_builder& feature_dynamic_rendering(feature feature);

        device_builder& collect_physical_devices(const instance& i, const vk::SurfaceKHR& surface);
        device_builder& get_physical_devices(eastl::vector<std::string_view>& devices);
        device_builder& set_preferred_physical_device(std::string name);

        device build(const instance& i);

    private:

        struct queue_data
        {
            std::uint32_t familyIndex = 0;
            std::uint32_t queueIndex = 0;
        };
        struct physical_device_data
        {
            vk::PhysicalDevice device = nullptr;
            std::string name{};
            std::uint32_t vulkanVersion = vk::ApiVersion10;
			vk::PhysicalDeviceType deviceType = vk::PhysicalDeviceType::eOther;
            vk::DeviceSize VRAM = 0;

            std::uint32_t score = 0;

            eastl::vector<const char*> extensions{};
			eastl::vector_map<queue_type, queue_data> queues{};
            location featureDynamicRendering = location::none;
        };

        eastl::vector<physical_device_data> m_physicalDevices;

        eastl::vector_set<const char*> m_requiredExtensions;
        std::string m_preferredDeviceName;
        std::uint32_t m_minVulkanVersion = vk::ApiVersion10;
        bool m_preferDedicatedTransferQueue = false;
        bool m_preferDedicatedComputeQueue = false;

        feature m_featureDynamicRendering = feature::none;


        [[nodiscard]] bool gather_physical_devices(const instance& i, const vk::SurfaceKHR& surface);
        void get_physical_device_data(physical_device_data& outData, const vk::PhysicalDevice& device,
            const vk::SurfaceKHR& surface) const;
        void get_physical_device_queues(physical_device_data& data,
            const vk::SurfaceKHR& surface) const;
        [[nodiscard]] bool get_physical_device_features(physical_device_data& data) const;

        void calculate_physical_devices_score();
    };
}