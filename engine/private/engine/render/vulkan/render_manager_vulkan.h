#pragma once

#include <engine/core.h>
#include <engine/render/vulkan/core.h>

#include <engine/render/render_manager.h>

namespace engine
{
    class render_manager_vulkan final : public render_manager
    {
        using super = render_manager;

    public:
        render_manager_vulkan() = default;
        virtual ~render_manager_vulkan() override = default;

        [[nodiscard]] static render_manager_vulkan* instance() { return s_instanceVulkan; }

        [[nodiscard]] const vulkan::context& vk_ctx() const { return m_ctx; }

    protected:

        virtual bool init(const create_info& info) override;
        virtual void clear() override;

    private:

        static render_manager_vulkan* s_instanceVulkan;

        vulkan::context m_ctx;
        vk::PhysicalDevice m_physicalDevice;

        [[nodiscard]] bool create_instance(const create_info& info);
        [[nodiscard]] bool create_device();
    };
}