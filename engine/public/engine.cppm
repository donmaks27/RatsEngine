module;

#include <engine_export.h>

export module rats_engine;

import rats_engine.render;

namespace engine
{
	export class RATS_ENGINE_EXPORT engine final
	{
		engine() = default;
		~engine();
	public:
		engine(const engine&) = delete;
		engine(engine&&) = delete;

		engine& operator=(const engine&) = delete;
		engine& operator=(engine&&) = delete;

		static engine& instance()
		{
			static engine engineInstance;
			return engineInstance;
		}

		bool start();
		[[nodiscard]] bool is_started() const { return m_engineStarted; }

	private:

		render_manager* m_renderManager = nullptr;

		bool m_engineStarted = false;


		bool init_engine();
		void clear_engine();
	};
}
