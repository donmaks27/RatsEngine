#include <engine/service1.h>

namespace engine
{
    utils::type_storage<service1_type> service1::ServiceTypes{};

    RATS_ENGINE_BASE_SERVICE1_IMPL(test_service)
    RATS_ENGINE_SERVICE1_IMPL(test_impl_service)
}
