#pragma once

#include <EASTL/finally.h>

#define RATS_ENGINE_DEFER_HELPER_ID(ID, Func) eastl::finally ratsEngineDeferObject_ ## ID = Func
#define RATS_ENGINE_DEFER_HELPER(ID, Func) RATS_ENGINE_DEFER_HELPER_ID(ID, Func)
#define RATS_ENGINE_DEFER(Func) RATS_ENGINE_DEFER_HELPER(__COUNTER__, Func)