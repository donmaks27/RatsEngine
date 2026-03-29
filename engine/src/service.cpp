#include <engine/service.h>

namespace engine
{
    utils::type_storage<service_type> service::ServiceTypes;
    eastl::array<service*, std::numeric_limits<service_type>::max()> service::ServiceInstances;
}