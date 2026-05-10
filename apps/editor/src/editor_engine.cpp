#include <editor/editor_engine.h>

namespace editor
{
    editor_engine* editor_engine::Instance = nullptr;

    editor_engine::editor_engine(engine::engine_config&& cfg)
        : runtime_engine(std::move(cfg))
    {
        Instance = this;
    }
    editor_engine::~editor_engine()
    {
        Instance = nullptr;
    }
}
