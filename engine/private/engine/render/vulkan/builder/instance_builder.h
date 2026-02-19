#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <EASTL/span.h>

namespace engine::vulkan
{
    class instance_builder final
    {
    public:
        instance_builder() = default;
        ~instance_builder() = default;

        instance_builder& set_application_name(std::string name);
        instance_builder& set_engine_name(std::string name);
        instance_builder& set_application_version(std::uint8_t major = 1, std::uint8_t minor = 0, std::uint8_t patch = 0);
        instance_builder& set_engine_version(std::uint8_t major = 1, std::uint8_t minor = 0, std::uint8_t patch = 0);

        instance_builder& add_required_extension(const char* extension);
        instance_builder& add_required_extensions(eastl::span<const char* const> extensions);
        instance_builder& set_max_vulkan_version(std::uint32_t version);

        instance build();

    private:

        std::string m_applicationName;
        std::string m_engineName;
        std::uint32_t m_applicationVersion = vk::makeApiVersion(0, 1, 0, 0);
        std::uint32_t m_engineVersion = vk::makeApiVersion(0, 1, 0, 0);

        eastl::vector<const char*> m_requiredExtensions;
        std::uint32_t m_maxVulkanVersion = vk::ApiVersion10;
    };
}