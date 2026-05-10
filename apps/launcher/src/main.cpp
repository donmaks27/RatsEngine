#include <game/game_engine.h>

int main()
{
	game::game_engine instance;
	if (!instance.start())
	{
		engine::log::fatal("[Game] Some error happened!");
		return -1;
	}
	return 0;
}
