#include <engine/runtime/runtime_engine.h>

namespace engine::runtime
{
    runtime_engine* runtime_engine::Instance = nullptr;

    runtime_engine::runtime_engine(engine_config&& cfg)
        : core_engine(std::move(cfg))
    {
        Instance = this;
    }
    runtime_engine::~runtime_engine()
    {
        Instance = nullptr;
    }
}
