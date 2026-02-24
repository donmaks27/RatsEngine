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
            vk::PhysicalDevice device;
            std::string name;
            vk::DeviceSize VRAM = 0;
            std::uint32_t vulkanVersion = vk::ApiVersion10;

            std::uint32_t score = 0;

            eastl::vector<const char*> extensions;
            queue_data graphicsQueue;
            queue_data transferQueue;
            location featureDynamicRendering = location::none;
        };

        eastl::vector<physical_device_data> m_physicalDevices;

        eastl::vector_set<const char*> m_requiredExtensions;
        std::string m_preferredDeviceName;
        std::uint32_t m_minVulkanVersion = vk::ApiVersion10;
        feature m_featureDynamicRendering = feature::none;

        [[nodiscard]] physical_device_data get_physical_device_data(const vk::PhysicalDevice& device,
            const vk::SurfaceKHR& surface) const;
        [[nodiscard]] static bool get_physical_device_queues(physical_device_data& data);
        [[nodiscard]] bool get_physical_device_features(physical_device_data& data) const;
    };
}