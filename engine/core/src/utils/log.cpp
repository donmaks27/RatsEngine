#include <engine/utils/log.h>
#include <engine/private_config_macro.h>

#if RATS_ENGINE_COLOR_LOGS
    #include <fmt/color.h>
#endif

namespace engine::log
{
    namespace
    {
        constexpr const char* typeToString(const type value)
        {
            switch (value)
            {
            case type::log:		return "[LOG]  ";
            case type::info:	return "[INFO] ";
            case type::warning:	return "[WARN] ";
            case type::error:	return "[ERROR]";
            case type::fatal:	return "[FATAL]";
            }
            return					   "       ";
        }
#if RATS_ENGINE_COLOR_LOGS
        constexpr fmt::color typeToColor(const type value)
        {
            switch (value)
            {
            case type::log:		return fmt::color::white;
            case type::info:	return fmt::color::lime_green;
            case type::warning:	return fmt::color::yellow;
            case type::error:	return fmt::color::red;
            case type::fatal:	return fmt::color::dark_red;
            }
            return fmt::color::gray;
        }
#endif
    }

    void print(const type t, std::string_view msg)
    {
#if RATS_ENGINE_COLOR_LOGS
        fmt::print(fmt::fg(typeToColor(t)), "{} {}\n", typeToString(t), msg);
#else
        fmt::print("{} {}\n", typeToString(t), msg);
#endif
    }
}
