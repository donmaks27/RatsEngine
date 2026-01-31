module;

#include <GLFW/glfw3.h>

export module rats_engine.render.glfw;

import rats_engine.render;
import rats_engine.utils;

export namespace engine
{
	class window_manager_glfw : public window_manager
	{
		using super = window_manager;

	protected:
		window_manager_glfw() = default;
		virtual ~window_manager_glfw() override = default;
	public:

		[[nodiscard]] virtual bool shouldCloseMainWindow() const override;
		virtual void on_frame_end() override;

	protected:

		virtual bool init(const create_info& info) override;
		virtual void clear() override;

	private:

		GLFWwindow* m_mainWindow = nullptr;


		void clear_GLFW();
	};
}