#pragma once

#include <engine/core.h>

#include <engine/utils/id.h>

namespace engine
{
    using surface_id = std::uint8_t;
    constexpr surface_id invalid_surface_id = utils::id<surface_id>::invalid_id;
}