#include <engine/render/vulkan/render_manager_vulkan.h>

#include <engine/render/window_manager.h>
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

		if (!create_instance(info))
		{
			log::fatal("[render_manager_vulkan::init] Failed to create Vulkan instance!");
			return false;
		}
    	if (!window_manager_vulkan::instance()->on_instance_created(m_ctx))
    	{
    		log::fatal("[render_manager_vulkan::init] Failed to create Vulkan surfaces!");
    		return false;
    	}

    	if (!create_device())
    	{
    		log::fatal("[render_manager_vulkan::init] Failed to create Vulkan device!");
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

	window_manager_vulkan* window_manager_vulkan::s_instanceVulkan = nullptr;

	void window_manager_vulkan::clear_vulkan()
	{
    	if (!m_windowDataVulkan.empty())
    	{
    		const auto& ctx = render_manager_vulkan::instance()->ctx();
    		for (const auto& [id, data] : m_windowDataVulkan)
    		{
    			ctx.i().destroySurfaceKHR(data.surface);
    		}
    		m_windowDataVulkan.clear();
    	}
	}

	bool window_manager_vulkan::on_instance_created(const vulkan_context& ctx)
    {
    	const auto windowManager = engine::window_manager::instance();
    	return std::ranges::all_of(windowManager->window_ids(), [this, &ctx](const window_id& id) {
    		const auto surface = create_surface(ctx, id);
    		if (surface == nullptr)
    		{
				log::error("[window_manager_vulkan::on_instance_created] Failed to create surface for window {}", id);
				return false;
			}
			m_windowDataVulkan.emplace(id, window_data_vulkan{ surface });
    		return true;
    	});
    }

	vk::SurfaceKHR window_manager_vulkan::surface(const window_id& id) const
	{
    	const auto iter = m_windowDataVulkan.find(id);
    	return iter != m_windowDataVulkan.end() ? iter->second.surface : nullptr;
	}

	void window_manager_vulkan::on_init()
	{
		s_instanceVulkan = this;
	}
	void window_manager_vulkan::on_clear()
	{
		s_instanceVulkan = nullptr;
	}

	void window_manager_vulkan::on_destroy_window(const window_id& id)
    {
    	const auto iter = m_windowDataVulkan.find(id);
    	if (iter != m_windowDataVulkan.end())
    	{
    		render_manager_vulkan::instance()->ctx().i().destroySurfaceKHR(iter->second.surface);
    		m_windowDataVulkan.erase(iter);
    	}
    }
}