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

namespace engine
{
	class render_manager_vulkan;
}

namespace engine::vulkan
{
    class instance_builder;
    class device_builder;

    class instance final
    {
        friend instance_builder;

    public:
        instance() = default;
        instance(std::nullptr_t) {}
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
        instance& operator=(std::nullptr_t)
        {
            clear();
            return *this;
		}

        [[nodiscard]] const vk::Instance* operator->() const { return &m_instance.get(); }
        [[nodiscard]] const vk::Instance& operator*() const { return m_instance.get(); }

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

    enum class queue_type : std::uint32_t { graphics, present, compute, transfer };
    class queue final
    {
        friend device_builder;

    public:
        queue() = default;
        queue(std::nullptr_t) {}
        queue(const queue&) = default;
        queue(queue&&) noexcept = default;
        ~queue() = default;

        queue& operator=(const queue&) = default;
        queue& operator=(queue&&) noexcept = default;
        queue& operator=(std::nullptr_t)
        {
            clear();
            return *this;
        }

        [[nodiscard]] const vk::Queue* operator->() const { return &m_queue; }
        [[nodiscard]] const vk::Queue& operator*() const { return m_queue; }

        [[nodiscard]] bool valid() const { return m_queue != nullptr; }
        [[nodiscard]] bool operator!=(std::nullptr_t) const { return valid(); }
        [[nodiscard]] bool operator==(std::nullptr_t) const { return !valid(); }

        void clear()
        {
            m_queue = nullptr;
            m_familyIndex = 0;
            m_queueIndex = 0;
        }

    private:

        vk::Queue m_queue = nullptr;
        std::uint32_t m_familyIndex = 0;
        std::uint32_t m_queueIndex = 0;
    };

    class device final
    {
        friend device_builder;

    public:
        device() = default;
        device(std::nullptr_t) {}
        device(const device&) = delete;
        device(device&&) noexcept = default;
        ~device() = default;

        device& operator=(const device&) = delete;
        device& operator=(device&&) noexcept = default;
        device& operator=(std::nullptr_t)
        {
            clear();
            return *this;
		}

        [[nodiscard]] const vk::Device* operator->() const { return &m_device.get(); }
        [[nodiscard]] const vk::Device& operator*() const { return m_device.get(); }

        [[nodiscard]] bool valid() const { return m_device.get() != nullptr; }
        [[nodiscard]] bool operator!=(std::nullptr_t) const { return valid(); }
        [[nodiscard]] bool operator==(std::nullptr_t) const { return !valid(); }

        void clear()
        {
            m_queues.clear();
            m_device.reset();
            m_physicalDevice = nullptr;
        }

    private:

        vk::PhysicalDevice m_physicalDevice;
        vk::UniqueDevice m_device;
        eastl::vector_map<queue_type, queue> m_queues;
    };

    class context final
    {
        friend render_manager_vulkan;

    public:
        context() = default;
        context(const context&) = delete;
        context(context&&) = delete;
        ~context() = default;

        context& operator=(const context&) = delete;
        context& operator=(context&&) = delete;

        [[nodiscard]] const instance& i() const { return m_instance; }
        [[nodiscard]] const device& d() const { return m_device; }

    private:

        instance m_instance;
        device m_device;
    };
}