#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <EASTL/span.h>
#include <EASTL/vector_set.h>

namespace engine::vulkan
{
    class instance_builder final
    {
    public:
        instance_builder() = default;
        instance_builder(const instance_builder&) = delete;
        instance_builder(instance_builder&&) = delete;
        ~instance_builder() = default;

        instance_builder& operator=(const instance_builder&) = delete;
        instance_builder& operator=(instance_builder&&) = delete;

        [[nodiscard]] instance_builder& set_application_name(std::string name);
        [[nodiscard]] instance_builder& set_engine_name(std::string name);
        [[nodiscard]] instance_builder& set_application_version(std::uint8_t major = 1, std::uint8_t minor = 0, std::uint8_t patch = 0);
        [[nodiscard]] instance_builder& set_engine_version(std::uint8_t major = 1, std::uint8_t minor = 0, std::uint8_t patch = 0);

        [[nodiscard]] instance_builder& add_required_extension(const char* extension);
        [[nodiscard]] instance_builder& add_required_extensions(eastl::span<const char* const> extensions);
        [[nodiscard]] instance_builder& set_max_vulkan_version(std::uint32_t version);

        [[nodiscard]] instance build();

    private:

        std::string m_applicationName;
        std::string m_engineName;
        std::uint32_t m_applicationVersion = vk::makeApiVersion(0, 1, 0, 0);
        std::uint32_t m_engineVersion = vk::makeApiVersion(0, 1, 0, 0);

        eastl::vector_set<const char*> m_requiredExtensions;
        std::uint32_t m_maxVulkanVersion = vk::ApiVersion10;
    };
}