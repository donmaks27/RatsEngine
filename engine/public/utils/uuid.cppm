module;

#include <engine_export.h>

#include <string>

#include <EASTL/functional.h>
#include <fmt/format.h>

export module rats_engine.utils:uuid;

export namespace engine::utils
{
	class RATS_ENGINE_EXPORT uuid
	{
	public:
		constexpr uuid() = default;
		constexpr uuid(const uuid&) = default;

		uuid& operator=(const uuid&) = default;

		[[nodiscard]] static constexpr uuid invalid() { return {}; }
		[[nodiscard]] static uuid generate();

		[[nodiscard]] constexpr bool operator==(const uuid& other) const { return (m_hi == other.m_hi) && (m_lo == other.m_lo); }
		[[nodiscard]] constexpr bool operator!=(const uuid& other) const { return !operator==(other); }
		[[nodiscard]] constexpr bool operator<(const uuid& other) const { return (m_hi < other.m_hi) || ((m_hi == other.m_hi) && (m_lo < other.m_lo)); }

		[[nodiscard]] constexpr bool valid() const { return (m_hi != 0) || (m_lo != 0); }
		[[nodiscard]] constexpr operator bool() const { return valid(); }

		[[nodiscard]] constexpr std::size_t hash() const { return m_hi ^ m_lo; }
		[[nodiscard]] std::string to_string() const;

	private:

		uint64_t m_hi = 0;
		uint64_t m_lo = 0;
	};

	struct RATS_ENGINE_EXPORT uuid_hash
	{
		[[nodiscard]] constexpr std::size_t operator()(const uuid& id) const noexcept
		{
			return id.hash();
		}
	};

	RATS_ENGINE_EXPORT constexpr std::string format_as(const uuid& id)
	{
		return id.to_string();
	}
}

export namespace eastl
{
	template<>
	struct RATS_ENGINE_EXPORT hash<engine::utils::uuid> : engine::utils::uuid_hash {};
}
