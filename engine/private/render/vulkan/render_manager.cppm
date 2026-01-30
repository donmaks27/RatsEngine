module;

export module rats_engine.render.vulkan:render_manager;

export import rats_engine.render;

export namespace engine
{
	class render_manager_vulkan final : public render_manager
	{
		using super = render_manager;

	public:
		render_manager_vulkan() = default;
		virtual ~render_manager_vulkan() override = default;

	protected:

		virtual bool init(const create_info& info) override;
	};
}
