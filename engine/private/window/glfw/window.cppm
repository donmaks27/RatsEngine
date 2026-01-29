module;

#include <GLFW/glfw3.h>

export module rats_engine.window.glfw;
export import rats_engine.window;

export namespace engine
{
	class window_manager_glfw : public window_manager
	{
	public:
		window_manager_glfw() = default;
		virtual ~window_manager_glfw() override { clear_GLFW(); }

		virtual bool shouldCloseMainWindow() const override;
		virtual void on_frame_end() override;

		virtual bool init_window_manager() override;
		virtual void clear_window_manager() override;

	private:

		GLFWwindow* m_mainWindow = nullptr;


		void clear_GLFW();
	};
}