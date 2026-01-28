module;

#include "engine_export.h"

#include <cstdint>
#include <format>

export module rats_engine.utils:log;

export namespace rats_engine
{
	namespace log
	{
		enum class RATS_ENGINE_EXPORT type : std::uint8_t { log, info, warning, error, fatal };

		RATS_ENGINE_EXPORT void print(type t, std::string_view msg);
		/*template<typename... Args>
		void print(const type t, std::format_string<Args...> fmtStr, Args&&... args)
		{
			log::print(t, std::format(fmtStr, std::forward<Args>(args)...));
		}*/

		/*enum class ENGINE_API type : std::uint8_t { log, info, warning, error, fatal };

	ENGINE_API void print(type t, std::string_view msg);
	template<typename... Args>
	void print(const type t, fmt::format_string<Args...> fmtStr, Args&&... args)
	{
		print(t, fmt::format(fmtStr, std::forward<Args>(args)...));
	}

	inline void log(const std::string_view msg) { print(type::log, msg); }
	template<typename... Args>
	void log(fmt::format_string<Args...> fmtStr, Args&&... args) { print(type::log, fmtStr, std::forward<Args>(args)...); }

	inline void info(const std::string_view msg) { print(type::info, msg); }
	template<typename... Args>
	void info(fmt::format_string<Args...> fmtStr, Args&&... args) { print(type::info, fmtStr, std::forward<Args>(args)...); }

	inline void warning(const std::string_view msg) { print(type::warning, msg); }
	template<typename... Args>
	void warning(fmt::format_string<Args...> fmtStr, Args&&... args) { print(type::warning, fmtStr, std::forward<Args>(args)...); }

	inline void error(const std::string_view msg) { print(type::error, msg); }
	template<typename... Args>
	void error(fmt::format_string<Args...> fmtStr, Args&&... args) { print(type::error, fmtStr, std::forward<Args>(args)...); }

	inline void fatal(const std::string_view msg) { print(type::fatal, msg); }
	template<typename... Args>
	void fatal(fmt::format_string<Args...> fmtStr, Args&&... args) { print(type::fatal, fmtStr, std::forward<Args>(args)...); }*/
	}
}
