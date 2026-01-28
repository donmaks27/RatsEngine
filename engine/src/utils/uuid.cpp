module;

#include <random>
#include <fmt/format.h>

module rats_engine.utils;

namespace engine::utils
{
	uuid uuid::generate()
	{
		static std::mt19937_64 generator(std::random_device{}());
		static std::uniform_int_distribution<uint64_t> distribution(0);

		uuid result;
		while (!result.valid())
		{
			result.m_hi = (distribution(generator) & 0xFFFFFFFFFFFF0FFFull) | 0x0000000000004000ull;
			result.m_lo = (distribution(generator) & 0x3FFFFFFFFFFFFFFFull) | 0x8000000000000000ull;
		}
		return result;
	}

	std::string uuid::to_string() const
	{
		const uint32_t a = m_hi >> 32;
		const uint32_t b = m_hi & 0xFFFFFFFF;
		const uint32_t c = m_lo >> 32;
		const uint32_t d = m_lo & 0xFFFFFFFF;
		return fmt::format("{:08x}-{:04x}-{:04x}-{:04x}-{:04x}{:08x}",
			a,
			b >> 16, b & 0xFFFF,
			c >> 16, c & 0xFFFF,
			d);
	}
}