module;

#include <print>

module rats_engine.utils;

namespace rats_engine::log
{
	void print(type t, std::string_view msg)
	{
		std::print("{}", msg);
	}
}
