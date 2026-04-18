#pragma once

#include <engine/core.h>

#include <EASTL/deque.h>

namespace engine::utils
{
    template<std::unsigned_integral Type>
    class id
    {
    public:
        id() = default;
        id(const id&) = default;
        ~id() = default;

        id& operator=(const id&) = default;

        using id_t = Type;
        static constexpr id_t invalid_id = std::numeric_limits<id_t>::max();

        [[nodiscard]] id_t generate()
        {
            if (!m_unusedIDs.empty())
            {
                id_t id = m_unusedIDs.front();
                m_unusedIDs.pop_front();
                return id;
            }
            if (m_nextID != invalid_id)
            {
                return m_nextID++;
            }
            return invalid_id;
        }
        void free(const id_t id)
        {
            if (std::ranges::find(m_unusedIDs, id) == m_unusedIDs.end())
            {
                m_unusedIDs.push_back(id);
            }
        }
        void reset()
        {
            m_unusedIDs.clear();
            m_nextID = 0;
        }

    private:

        eastl::deque<id_t> m_unusedIDs;
        Type m_nextID = 0;
    };
}
