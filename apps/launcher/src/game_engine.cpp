#include <game/game_engine.h>

namespace game
{
    game_engine* game_engine::Instance = nullptr;

    game_engine::game_engine(engine::engine_config&& cfg)
        : runtime_engine(std::move(cfg))
    {
        Instance = this;
    }
    game_engine::~game_engine()
    {
        Instance = nullptr;
    }
}
