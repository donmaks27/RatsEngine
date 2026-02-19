#pragma once

#include <fmt/format.h>
#include <vulkan/vulkan.hpp>

template<>
struct fmt::formatter<vk::Result> : formatter<std::string>
{
    template<typename FormatContext>
    auto format(const vk::Result result, FormatContext& ctx) const
    {
        const auto value = fmt::format("{} (0x{:08X})", vk::to_string(result), static_cast<std::uint32_t>(result));
        return formatter<std::string>::format(value, ctx);
    }
};

namespace engine::vulkan
{
    class instance_builder;

    class instance final
    {
        friend instance_builder;

    public:
        instance() = default;
        instance(const instance&) = delete;
        instance(instance&& value) noexcept = default;
        ~instance() { clear(); }

        instance& operator=(const instance&) = delete;
        instance& operator=(instance&& value) noexcept
        {
            m_debugMessenger = std::move(value.m_debugMessenger);
            m_instance = std::move(value.m_instance);
            return *this;
        }

        [[nodiscard]] const vk::Instance* operator->() const { return &m_instance.get(); }
        [[nodiscard]] const vk::Instance& operator*() const { return m_instance.get(); }
        [[nodiscard]] const vk::Instance& value() const { return m_instance.get(); }

        [[nodiscard]] bool valid() const { return m_instance.get() != nullptr; }
        [[nodiscard]] bool operator!=(std::nullptr_t) const { return  valid(); }
        [[nodiscard]] bool operator==(std::nullptr_t) const { return !valid(); }

        void clear()
        {
            m_debugMessenger.reset();
            m_instance.reset();
        }

    private:

        vk::UniqueInstance m_instance;
        vk::UniqueDebugUtilsMessengerEXT m_debugMessenger;
    };
}