module;

module rats_engine.render.vulkan;

namespace engine
{
	render_manager* render_manager::create_instance_impl_vulkan(const create_info& info)
	{
		return new render_manager_vulkan();
	}

	bool render_manager_vulkan::init(const create_info& info)
	{
		return super::init(info);
	}
}
