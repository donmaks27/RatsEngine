#include <engine/service.h>

#include <cassert>

namespace engine
{
    utils::type_storage<service_type> service::ServiceTypes;

    namespace registration
    {
        service::service(const service_type type)
            : m_type(type)
        {
            assert(type != invalid_service_type);
        }

        service& service_storage::register_service(const service_type type)
        {
            const auto iter = m_services.find(type);
            assert(m_services.find(type) == m_services.end());
            return iter != m_services.end() ? iter->second : m_services.emplace(type, type).first->second;
        }
    }
}
