#pragma once

#include <engine/engine_export.h>

#include <new>

RATS_ENGINE_EXPORT void* operator new[](std::size_t size, const char* pName, int flags, unsigned debugFlags,
    const char* file, int line);
RATS_ENGINE_EXPORT void* operator new[](std::size_t size, std::size_t alignment, std::size_t offset, const char* pName,
    int flags, unsigned int debugFlags, const char* file, int line);