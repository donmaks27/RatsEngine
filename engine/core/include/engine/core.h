#pragma once

#include <engine/engine_export.h>
#include <engine/config.h>
#include <engine/eastl_config.h>

#include <engine/utils/log.h>

#include <cassert>

namespace engine
{
	[[nodiscard]] constexpr log::logger logger_engine() { return log::logger("engine"); }
}

#define RATS_ENGINE_ASSERT(expr) assert(expr)
