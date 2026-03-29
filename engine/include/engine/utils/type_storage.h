#pragma once

#include <EASTL/hash_map.h>

namespace engine::utils
{
    template<typename TypeId> requires std::unsigned_integral<TypeId>
    class type_storage final
    {
    public:
        type_storage() = default;
        type_storage(const type_storage&) = delete;
        type_storage(type_storage&& value) noexcept
            : m_types(std::move(value.m_types))
            , m_nextId(value.m_nextId)
        {
            value.m_nextId = 0;
        }
        ~type_storage() = default;

        type_storage& operator=(const type_storage&) = delete;
        type_storage& operator=(type_storage&& value) noexcept
        {
            m_types = std::move(value.m_types);
            m_nextId = value.m_nextId;
            value.m_nextId = 0;
            return *this;
        }

        using type_id = TypeId;
        static constexpr type_id invalid_id = std::numeric_limits<type_id>::max();

        template<typename T>
        [[nodiscard]] type_id get_type_id() { return get_type_id(typeid(T)); }

        void clear()
        {
            m_types.clear();
            m_nextId = 0;
        }

    private:

        struct type_hash
        {
            [[nodiscard]] size_t operator()(const std::type_info* value) const
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

        eastl::hash_map<const std::type_info*, type_id, type_hash, type_compare> m_types;
        type_id m_nextId = 0;

        type_id get_type_id(const std::type_info& type)
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
