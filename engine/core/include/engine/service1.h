#pragma once

#include <engine/core.h>

#include <engine/utils/log.h>
#include <engine/utils/type_storage.h>

namespace engine
{
    class engine;
    class service1;

    using service1_type = std::uint8_t;
    constexpr service1_type invalid_service_type = utils::type_storage<service1_type>::invalid_id;

    template<typename T>
    concept service1_class = std::derived_from<T, service1>;
    template<typename T>
    concept base_service1_class = service1_class<T> && std::is_same_v<typename T::super_t, service1>;

    class service1
    {
    protected:
        service1() = default;
        virtual ~service1() = default;
    public:
        service1(const service1&) = delete;
        service1(service1&&) = delete;

        service1& operator=(const service1&) = delete;
        service1& operator=(service1&&) = delete;

        using this_t = service1;

    protected:

        template<typename T> requires base_service1_class<T>
        [[nodiscard]] static service1_type type() { return ServiceTypes.type_id<T>(); }
        template<typename T> requires service1_class<T>
        [[nodiscard]] static log::logger logger(const std::string_view category)
        {
            if constexpr (base_service1_class<T>)
            {
                return { category, log::logger("service") };
            }
            else
            {
                return { category, T::super_t::instance().Log };
            }
        }

        virtual bool service_init() = 0;
        virtual void service_clear() = 0;

    private:

        static utils::type_storage<service1_type> ServiceTypes;
    };

#define RATS_ENGINE_SERVICE1(ServiceType, LogCategory)\
        friend service1;\
    protected:\
        ServiceType() { Instance = this; }\
        virtual ~ServiceType() override { Instance = nullptr; }\
    public:\
        ServiceType(const ServiceType&) = delete;\
        ServiceType(ServiceType&&) = delete;\
        ServiceType& operator=(const ServiceType&) = delete;\
        ServiceType& operator=(ServiceType&&) = delete;\
        using super_t = this_t;\
        using this_t = ServiceType;\
        [[nodiscard]] static auto& instance() { return *Instance; }\
    protected:\
        const log::logger Log = service1::logger<this_t>(LogCategory);\
    private:\
        static this_t* Instance;

#define RATS_ENGINE_BASE_SERVICE1(ServiceType, LogCategory)\
        RATS_ENGINE_SERVICE1(ServiceType, LogCategory)\
    public:\
        [[nodiscard]] static service1_type type();\
    private:

#define RATS_ENGINE_SERVICE1_IMPL(ServiceType)\
    ServiceType* ServiceType::Instance = nullptr;

#define RATS_ENGINE_BASE_SERVICE1_IMPL(ServiceType)\
    RATS_ENGINE_SERVICE1_IMPL(ServiceType)\
    service1_type ServiceType::type()\
    {\
        static auto serviceType = service1::type<ServiceType>();\
        return serviceType;\
    }

    class test_service : public service1
    {
        RATS_ENGINE_BASE_SERVICE1(test_service, "test")
    };

    class test_impl_service : public test_service
    {
        RATS_ENGINE_SERVICE1(test_impl_service, "impl")
    };
}
