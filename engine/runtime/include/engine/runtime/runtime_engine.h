#pragma once

#include <engine/runtime/core.h>
#include <engine/core_engine.h>

namespace engine::runtime
{
    class RATS_ENGINE_RUNTIME_EXPORT runtime_engine : public core_engine
    {
    protected:
        explicit runtime_engine(engine_config&& cfg);
        virtual ~runtime_engine() override;
    public:
        runtime_engine() = delete;
        runtime_engine(const runtime_engine&) = delete;
        runtime_engine(runtime_engine&&) = delete;

        runtime_engine& operator=(const runtime_engine&) = delete;
        runtime_engine& operator=(runtime_engine&&) = delete;

        [[nodiscard]] static auto& instance() { return *Instance; }

    private:

        static runtime_engine* Instance;
    };
}
