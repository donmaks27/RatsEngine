#pragma once

#include <typeinfo>
#include <EASTL/unordered_map.h>

namespace engine::utils
{
    template<typename TypeId> requires std::unsigned_integral<TypeId>
    class type_storage final
    {
    public:
        type_storage() = default;
        type_storage(const type_storage&) = delete;
        type_storage(type_storage&&) = delete;
        ~type_storage() = default;

        type_storage& operator=(const type_storage&) = delete;
        type_storage& operator=(type_storage&&) = delete;

        using id = TypeId;
        static constexpr id invalid_id = std::numeric_limits<id>::max();

        template<typename T>
        [[nodiscard]] id type_id() { return type_id(typeid(T)); }

    private:

        struct type_hash
        {
            [[nodiscard]] std::size_t operator()(const std::type_info* value) const
            {
                return value->hash_code();
            }
        };
        struct type_compare
        {
            [[nodiscard]] bool operator()(const std::type_info* lhs, const std::type_info* rhs) const
            {
                return *lhs == *rhs;
            }
        };

        eastl::unordered_map<const std::type_info*, id, type_hash, type_compare> m_types;
        id m_nextId = 0;

        id type_id(const std::type_info& type)
        {
            const auto iter = m_types.find(&type);
            if (iter != m_types.end())
            {
                return iter->second;
            }
            if (m_nextId == invalid_id)
            {
                return invalid_id;
            }
            return m_types[&type] = m_nextId++;
        }
    };
}
