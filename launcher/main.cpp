import rats_engine;
import rats_engine.utils;

int main()
{
	if (!engine::engine::instance().start())
	{
		engine::log::fatal("Engine failed to start!");
		return -1;
	}
	return 0;
}
