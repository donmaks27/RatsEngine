#include <engine/render/vulkan/render_manager_vulkan.h>

#include <engine/render/vulkan/window_manager_vulkan.h>
#include <engine/render/vulkan/builder/instance_builder.h>
#include <engine/render/vulkan/builder/device_builder.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace engine
{
    namespace
	{
    	constexpr eastl::array RequiredInstanceExtensions = { vk::KHRGetSurfaceCapabilities2ExtensionName };
    	constexpr eastl::array RequiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
    	constexpr auto MinVulkanApiVersion = vk::ApiVersion11;
	}

	render_manager_vulkan* render_manager_vulkan::s_instanceVulkan = nullptr;

	bool render_manager_vulkan::init(const create_info& info)
	{
    	s_instanceVulkan = this;

		if (!super::init(info))
		{
			return false;
		}

		auto* windowManager = window_manager_vulkan::instance();
		if (!create_instance(info))
		{
			log::fatal("[render_manager_vulkan::init] Failed to create Vulkan instance!");
			return false;
		}
    	if (!windowManager->on_instance_created())
    	{
    		return false;
    	}
    	if (!create_device())
    	{
    		log::fatal("[render_manager_vulkan::init] Failed to create Vulkan device!");
    		return false;
    	}
		if (!windowManager->on_device_created())
		{
			return false;
		}

		return true;
	}

	void render_manager_vulkan::clear()
	{
		if (m_ctx.m_instance != nullptr)
		{
			window_manager_vulkan::instance()->clear_vulkan();

            m_ctx.m_device.clear();
			m_ctx.m_instance.clear();
		}

    	s_instanceVulkan = nullptr;
		super::clear();
	}

	bool render_manager_vulkan::create_instance(const create_info& info)
	{
		auto instance = vulkan::instance_builder()
			.set_application_name(info.appName)
			.set_engine_name("RatsEngine")
			.set_application_version(0, 1, 0)
			.set_engine_version(0, 1, 0)
			.add_required_extensions(window_manager_vulkan::instance()->required_instance_extensions())
			.add_required_extensions(RequiredInstanceExtensions)
			.set_max_vulkan_version(vk::ApiVersion13)
			.build();
		if (instance == nullptr)
		{
			return false;
		}
		m_ctx.m_instance = std::move(instance);
		return true;
	}

	bool render_manager_vulkan::create_device()
    {
    	const auto mainSurface = window_manager_vulkan::instance()->surface(window_manager::instance()->main_window_id());
        auto device = vulkan::device_builder()
            .add_required_extensions(RequiredDeviceExtensions)
            .set_min_vulkan_version(MinVulkanApiVersion)
            .feature_dynamic_rendering(vulkan::device_builder::feature::required)
            .collect_physical_devices(m_ctx.m_instance, mainSurface)
            .build(m_ctx.m_instance);
        if (device == nullptr)
        {
            return false;
        }
        m_ctx.m_device = std::move(device);
        return true;
	}
}