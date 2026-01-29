module;

#include <engine_export.h>

export module rats_engine.window;

export import rats_engine.utils;

export namespace engine
{
	class RATS_ENGINE_EXPORT window_manager
	{
	public:
		window_manager() = default;
		virtual ~window_manager() = default;
	public:
		window_manager(const window_manager&) = delete;
		window_manager(window_manager&&) = delete;

		window_manager& operator=(const window_manager&) = delete;
		window_manager& operator=(window_manager&&) = delete;

		[[nodiscard]] virtual bool shouldCloseMainWindow() const { return true; }
		virtual void on_frame_end() {}

		[[nodiscard]] virtual bool init_window_manager() { return false; }
		virtual void clear_window_manager() {}
	};
}
