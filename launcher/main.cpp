import rats_engine;

int main()
{
	if (!engine::engine::instance().start())
	{
		engine::log::fatal("Engine failed to start!");
		return -1;
	}
	return 0;
}
