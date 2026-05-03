#include <engine/engine.h>
#include <engine/utils/log.h>

int main()
{
	if (!engine::engine::instance().start())
	{
		engine::log::fatal("Engine failed to start!");
		return -1;
	}
	return 0;
}
