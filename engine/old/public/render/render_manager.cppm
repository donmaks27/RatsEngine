module;

#include <engine_export.h>
#include <string>

export module rats_engine.render:render_manager;

import :render_api;
import :window_manager;

export namespace engine
{
	class RATS_ENGINE_EXPORT render_manager
	{
	protected:
		render_manager() = default;
		virtual ~render_manager() = default;
	public:
		render_manager(const render_manager&) = delete;
		render_manager(render_manager&&) = delete;

		render_manager& operator=(const render_manager&) = delete;
		render_manager& operator=(render_manager&&) = delete;

		struct create_info
		{
			std::string appName = "RatsEngine";

			render_api api = render_api::vulkan;
		};
		static render_manager* create_instance(const create_info& info);
		[[nodiscard]] static render_manager* instance() { return s_instance; }
		static void clear_instance();

	protected:

		[[nodiscard]] virtual bool init(const create_info& info);
		virtual void clear();

	private:

		static render_manager* s_instance;
		static render_manager* create_instance_impl(const create_info& info);
		static render_manager* create_instance_impl_vulkan(const create_info& info);
	};
}
