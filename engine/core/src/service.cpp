#include <engine/service.h>

namespace engine
{
    utils::type_storage<service_type> service::ServiceTypes;

    namespace registration
    {
        service::service(const service_type type)
            : m_type(type)
        {
            RATS_ENGINE_ASSERT(type != invalid_service_type);
        }

        service& service_storage::register_service(const service_type type)
        {
            const auto iter = m_services.find(type);
            RATS_ENGINE_ASSERT(iter == m_services.end());
            return iter != m_services.end() ? iter->second : m_services.emplace(type, type).first->second;
        }
    }
}
