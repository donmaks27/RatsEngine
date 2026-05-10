#pragma once

#include <game/core.h>
#include <engine/runtime/runtime_engine.h>

namespace game
{
    class game_engine final : public engine::runtime::runtime_engine
    {
    public:
        explicit game_engine(engine::engine_config&& cfg = {});
        game_engine(const game_engine&) = delete;
        game_engine(game_engine&&) = delete;
        virtual ~game_engine() override;

        game_engine& operator=(const game_engine&) = delete;
        game_engine& operator=(game_engine&&) = delete;

        [[nodiscard]] static auto& instance() { return *Instance; }

    private:

        static game_engine* Instance;
    };
}
