module;

#include <engine_export.h>

export module rats_engine.render:window_manager;

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

		struct create_info {};
		static window_manager* instance(const create_info& info);
		static void clear_instance();

		[[nodiscard]] virtual bool shouldCloseMainWindow() const { return true; }
		virtual void on_frame_end() {}

	protected:

		[[nodiscard]] virtual bool init(const create_info& info) { return true; }
		virtual void clear() {}

	private:

		static window_manager* s_instance;

		static window_manager* create_instance_impl(const create_info& info);
	};
}
