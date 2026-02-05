#pragma once

#include <engine/core.h>

#include <fmt/format.h>

namespace engine::log
{
    enum class type : std::uint8_t { log, info, warning, error, fatal };

    RATS_ENGINE_EXPORT void print(type t, std::string_view msg);
    template<typename... Args>
    void print(const type t, fmt::format_string<Args...> fmtStr, Args&&... args)
    {
        log::print(t, fmt::format(fmtStr, std::forward<Args>(args)...));
    }

    inline void log(const std::string_view msg) { log::print(type::log, msg); }
    template<typename... Args>
    void log(fmt::format_string<Args...> fmtStr, Args&&... args) { log::print(type::log, fmtStr, std::forward<Args>(args)...); }

    inline void info(const std::string_view msg) { log::print(type::info, msg); }
    template<typename... Args>
    void info(fmt::format_string<Args...> fmtStr, Args&&... args) { log::print(type::info, fmtStr, std::forward<Args>(args)...); }

    inline void warning(const std::string_view msg) { log::print(type::warning, msg); }
    template<typename... Args>
    void warning(fmt::format_string<Args...> fmtStr, Args&&... args) { log::print(type::warning, fmtStr, std::forward<Args>(args)...); }

    inline void error(const std::string_view msg) { log::print(type::error, msg); }
    template<typename... Args>
    void error(fmt::format_string<Args...> fmtStr, Args&&... args) { log::print(type::error, fmtStr, std::forward<Args>(args)...); }

    inline void fatal(const std::string_view msg) { log::print(type::fatal, msg); }
    template<typename... Args>
    void fatal(fmt::format_string<Args...> fmtStr, Args&&... args) { log::print(type::fatal, fmtStr, std::forward<Args>(args)...); }
}