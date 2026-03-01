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
            clear(render_manager_vulkan::instance()->vk_ctx());
        }
    }
    void command_pool::clear(const context& ctx)
    {
        if (valid())
        {
            ctx.d()->destroyCommandPool(m_value);
            m_value = nullptr;
        }
    }

    eastl::vector<vk::CommandBuffer> command_pool::command_buffers(const context& ctx, const std::uint32_t count,
        const bool primary) const
    {
        if (!valid() || (count == 0))
        {
            return {};
        }
        auto buffers = ctx.d()->allocateCommandBuffers({
            value(),
            primary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary,
            count
        });
        if (buffers.result != vk::Result::eSuccess)
        {
            log::warning("[vulkan::command_pool::command_buffers] Failed to allocate command buffers: {}", buffers.result);
            return {};
        }
        eastl::vector<vk::CommandBuffer> result;
        result.reserve(buffers.value.size());
        std::ranges::move(buffers.value, std::back_inserter(result));
        return result;
    }
    vk::CommandBuffer command_pool::command_buffer(const context& ctx, const bool primary) const
    {
        if (!valid())
        {
            return {};
        }
        auto buffer = ctx.d()->allocateCommandBuffers({
            value(),
            primary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary,
            1
        });
        if (buffer.result != vk::Result::eSuccess)
        {
            log::warning("[vulkan::command_pool::command_buffer] Failed to allocate command buffer: {}", buffer.result);
            return {};
        }
        return buffer.value[0];
    }

    vulkan::command_pool queue::command_pool(const context& ctx, const vk::CommandPoolCreateFlags flags) const
    {
        if (!valid())
        {
            return nullptr;
        }
        const auto commandPool = ctx.d()->createCommandPool({ flags, family_index() });
        if (commandPool.result != vk::Result::eSuccess)
        {
            log::error("[vulkan::queue::command_pool] Failed to create command pool: {}", commandPool.result);
            return nullptr;
        }
        vulkan::command_pool result;
        result.m_value = commandPool.value;
        return result;
    }
}
