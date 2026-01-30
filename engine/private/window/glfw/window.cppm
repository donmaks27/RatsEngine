module;

#include <GLFW/glfw3.h>

export module rats_engine.glfw;

import rats_engine;

export namespace engine
{
	class window_manager_glfw : public window_manager
	{
	public:
		window_manager_glfw() = default;
		virtual ~window_manager_glfw() override = default;

		virtual bool shouldCloseMainWindow() const override;
		virtual void on_frame_end() override;

	protected:

		virtual bool init_window_manager_impl() override;
		virtual void clear_window_manager_impl() override;

	private:

		GLFWwindow* m_mainWindow = nullptr;


		void clear_GLFW();
	};
}