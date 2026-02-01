module;

#include <engine_export.h>

export module rats_engine.render:window_manager;

import :render_api;

export namespace engine
{
	class RATS_ENGINE_EXPORT window_manager
	{
	protected:
		window_manager() = default;
		virtual ~window_manager() = default;
	public:
		window_manager(const window_manager&) = delete;
		window_manager(window_manager&&) = delete;

		window_manager& operator=(const window_manager&) = delete;
		window_manager& operator=(window_manager&&) = delete;

		struct create_info
		{
			render_api api = render_api::vulkan;
		};
		static window_manager* create_instance(const create_info& info);
		[[nodiscard]] static window_manager* instance() { return s_instance; }
		static void clear_instance();

		[[nodiscard]] virtual bool shouldCloseMainWindow() const { return true; }
		virtual void on_frame_end() {}

	protected:

		[[nodiscard]] virtual bool init(const create_info& info) { return true; }
		virtual void clear() {}

	private:

		static window_manager* s_instance;

		static window_manager* create_instance_impl(const create_info& info);
		static window_manager* create_instance_impl_vulkan(const create_info& info);
	};
}
