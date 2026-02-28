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

    template<typename AccessType, typename StoreType = AccessType>
    class _value_wrapper
    {
    public:
        _value_wrapper() = default;
        _value_wrapper(const _value_wrapper&) = delete;
        _value_wrapper(_value_wrapper&& value) noexcept = default;
        ~_value_wrapper() = default;

        _value_wrapper& operator=(const _value_wrapper&) = delete;
        _value_wrapper& operator=(_value_wrapper&& value) noexcept = default;

        [[nodiscard]] const auto* operator->() const { return &operator*(); }
        [[nodiscard]] const auto& operator*() const
        {
            if constexpr (!std::is_same_v<StoreType, AccessType>)
            {
                return m_value.get();
            }
            else
            {
                return m_value;
            }
        }

        [[nodiscard]] bool valid() const { return operator*() != nullptr; }
        [[nodiscard]] bool operator!=(std::nullptr_t) const { return  valid(); }
        [[nodiscard]] bool operator==(std::nullptr_t) const { return !valid(); }

    protected:

        StoreType m_value;
    };

    class instance final : public _value_wrapper<vk::Instance, vk::UniqueInstance>
    {
        friend instance_builder;

    public:
        instance() = default;
        instance(std::nullptr_t) {}
        instance(const instance&) = delete;
        instance(instance&& value) noexcept = default;
        ~instance() { clear(); }

        instance& operator=(const instance&) = delete;
        instance& operator=(instance&& value) noexcept = default;
        instance& operator=(std::nullptr_t)
        {
            clear();
            return *this;
		}

        void clear()
        {
            m_debugMessenger.reset();
            m_value.reset();
        }

    private:

        vk::UniqueDebugUtilsMessengerEXT m_debugMessenger;
    };

    class queue final : public _value_wrapper<vk::Queue>
    {
        friend device_builder;

    public:
        queue() = default;
        queue(std::nullptr_t) {}
        queue(const queue&) = delete;
        queue(queue&&) noexcept = default;
        ~queue() = default;

        queue& operator=(const queue&) = delete;
        queue& operator=(queue&&) noexcept = default;
        queue& operator=(std::nullptr_t)
        {
            clear();
            return *this;
        }

		[[nodiscard]] std::uint32_t family_index() const { return m_familyIndex; }
		[[nodiscard]] std::uint32_t queue_index() const { return m_queueIndex; }

        void clear()
        {
            m_value = nullptr;
            m_familyIndex = 0;
            m_queueIndex = 0;
        }

    private:

        std::uint32_t m_familyIndex = 0;
        std::uint32_t m_queueIndex = 0;
    };

    enum class queue_type : std::uint32_t { graphics, present, compute, transfer };
    class device final : public _value_wrapper<vk::Device, vk::UniqueDevice>
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

        [[nodiscard]] const vk::PhysicalDevice& physical_device() const { return m_physicalDevice; }
        [[nodiscard]] const vulkan::queue& queue(const queue_type type) const { return m_queues.at_key(type); }

        void clear()
        {
            m_queues.clear();
            m_physicalDevice = nullptr;
            m_value.reset();
        }

    private:

        vk::PhysicalDevice m_physicalDevice;
        eastl::vector_map<queue_type, vulkan::queue> m_queues;
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