#include <engine/render/vulkan/core.h>

#include <engine/render/vulkan/render_vulkan_service.h>

namespace engine::vulkan
{
	namespace 
	{
        const auto& Log = logger_render_api<render_api::vulkan>();
	}

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
            clear(render_vulkan_service::instance().vk_ctx());
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

    eastl::vector<vk::CommandBuffer> command_pool::allocate(const context& ctx, const std::uint32_t count,
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
            Log.warning("command_pool. Failed to allocate command buffers: {}", buffers.result);
            return {};
        }
        eastl::vector<vk::CommandBuffer> result;
        result.reserve(buffers.value.size());
        std::ranges::move(buffers.value, std::back_inserter(result));
        return result;
    }
    vk::CommandBuffer command_pool::allocate(const context& ctx, const bool primary) const
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
            Log.warning("command_pool. Failed to allocate command buffer: {}", buffer.result);
            return {};
        }
        return buffer.value[0];
    }

    void command_pool::reset(const context& ctx, const vk::CommandPoolResetFlags flags) const
    {
	    if (valid())
	    {
		    ctx.d()->resetCommandPool(value(), flags);
	    }
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
            Log.error("queue. Failed to create command pool: {}", commandPool.result);
            return nullptr;
        }
        vulkan::command_pool result;
        result.m_value = commandPool.value;
        return result;
    }

    swapchain::swapchain(swapchain&& other) noexcept
    {
	    m_value = other.m_value;
	    m_images = std::move(other.m_images);
	    m_imageIndex = other.m_imageIndex;
	    m_outdated = other.m_outdated;
	    other.m_value = nullptr;
	    other.m_images.clear();
	    other.m_imageIndex = std::numeric_limits<std::uint8_t>::max();
	    other.m_outdated = false;
    }

    swapchain& swapchain::operator=(swapchain&& other) noexcept
    {
	    clear();
	    m_value = other.m_value;
	    m_images = std::move(other.m_images);
	    m_imageIndex = other.m_imageIndex;
	    m_outdated = other.m_outdated;
	    other.m_value = nullptr;
	    other.m_images.clear();
	    other.m_imageIndex = std::numeric_limits<std::uint8_t>::max();
	    other.m_outdated = false;
	    return *this;
    }

    void swapchain::clear()
    {
	    if (valid())
	    {
	        clear(render_vulkan_service::instance().vk_ctx());
	    }
    }
    void swapchain::clear(const context& ctx)
    {
	    if (valid())
	    {
	        ctx.d()->waitIdle();

	        std::ranges::for_each(m_images, [&ctx](const image_data& data) {
	            ctx.d()->destroyImageView(data.imageView);
	            ctx.d()->destroySemaphore(data.renderFinishedSemaphore);
	        });
	        ctx.d()->destroySwapchainKHR(m_value);

	        m_value = nullptr;
	        m_images.clear();
	    	m_imageIndex = std::numeric_limits<std::uint8_t>::max();
	        m_outdated = false;
	    }
    }
}
