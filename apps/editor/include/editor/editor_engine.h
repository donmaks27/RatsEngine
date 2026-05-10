#pragma once

#include <editor/core.h>
#include <engine/runtime/runtime_engine.h>

namespace editor
{
    class editor_engine final : public engine::runtime::runtime_engine
    {
    public:
        explicit editor_engine(engine::engine_config&& cfg = {});
        editor_engine(const editor_engine&) = delete;
        editor_engine(editor_engine&&) = delete;
        virtual ~editor_engine() override;

        editor_engine& operator=(const editor_engine&) = delete;
        editor_engine& operator=(editor_engine&&) = delete;

        [[nodiscard]] static auto& instance() { return *Instance; }

    private:

        static editor_engine* Instance;
    };
}
