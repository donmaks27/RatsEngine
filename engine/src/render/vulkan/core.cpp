#include <engine/render/vulkan/core.h>

#include <engine/render/vulkan/render_manager_vulkan.h>

namespace engine::vulkan
{
    command_pool::command_pool(command_pool&& value) noexcept
    {
        m_value = value.m_value;
        value.m_value = nullptr;
    }
    command_pool& command_pool::operator=(command_pool&& value) noexcept
    {
        clear();
        m_value = value.m_value;
        value.m_value = nullptr;
        return *this;
    }
    command_pool& command_pool::operator=(std::nullptr_t)
    {
        clear();
        return *this;
    }
    void command_pool::clear()
    {
        if (valid())
        {
            const auto& device = render_manager_vulkan::instance()->vk_ctx().d();
            device->destroyCommandPool(m_value);
            m_value = nullptr;
        }
    }

    command_pool queue::create_command_pool(const vk::CommandPoolCreateFlags flags) const
    {
        if (!valid())
        {
            return nullptr;
        }
        const auto& device = render_manager_vulkan::instance()->vk_ctx().d();
        const auto commandPool = device->createCommandPool({ flags, family_index() });
        if (commandPool.result != vk::Result::eSuccess)
        {
            log::error("[vulkan::queue::create_command_pool] Failed to create command pool: {}", commandPool.result);
            return nullptr;
        }
        command_pool result;
        result.m_value = commandPool.value;
        return result;
    }
}
