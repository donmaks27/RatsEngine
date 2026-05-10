#pragma once

#include <engine/core.h>

#include <engine/utils/type_storage.h>

namespace engine
{
    class core_engine;
    class service;

    using service_type = std::uint8_t;
    constexpr service_type invalid_service_type = utils::type_storage<service_type>::invalid_id;

    template<typename T>
    concept service_class = std::derived_from<T, service>;
    template<typename T>
    concept base_service_class = service_class<T> && std::is_same_v<typename T::super_t, service>;

    class RATS_ENGINE_EXPORT service
    {
        friend core_engine;

    protected:
        service() = default;
        virtual ~service() = default;
    public:
        service(const service&) = delete;
        service(service&&) = delete;

        service& operator=(const service&) = delete;
        service& operator=(service&&) = delete;

        using this_t = service;

        [[nodiscard]] static constexpr auto logger() { return log::logger("service"); }

    protected:

        template<typename T> requires base_service_class<T>
        [[nodiscard]] static service_type type() { return ServiceTypes.type_id<T>(); }
        template<typename T> requires service_class<T>
        [[nodiscard]] static log::logger logger(const std::string_view category)
        {
            if constexpr (base_service_class<T>)
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

        static utils::type_storage<service_type> ServiceTypes;
    };
}

#define RATS_ENGINE_SERVICE(ServiceType, LogCategory)                   \
        friend service;                                                 \
    protected:                                                          \
        ServiceType() { Instance = this; }                              \
        virtual ~ServiceType() override { Instance = nullptr; }         \
    public:                                                             \
        ServiceType(const ServiceType&) = delete;                       \
        ServiceType(ServiceType&&) = delete;                            \
        ServiceType& operator=(const ServiceType&) = delete;            \
        ServiceType& operator=(ServiceType&&) = delete;                 \
        using super_t = this_t;                                         \
        using this_t = ServiceType;                                     \
        [[nodiscard]] static auto& instance() { return *Instance; }     \
    protected:                                                          \
        const log::logger Log = service::logger<this_t>(LogCategory);   \
    private:                                                            \
        static ServiceType* Instance;

#define RATS_ENGINE_BASE_SERVICE(ServiceType, LogCategory)                  \
        RATS_ENGINE_SERVICE(ServiceType, LogCategory)                       \
    public:                                                                 \
        [[nodiscard]] static service_type type();                           \
        [[nodiscard]] static const char* type_name() { return LogCategory; }\
    private:

#define RATS_ENGINE_SERVICE_IMPL(ServiceType) ServiceType* ServiceType::Instance = nullptr;

#define RATS_ENGINE_BASE_SERVICE_IMPL(ServiceType)          \
    RATS_ENGINE_SERVICE_IMPL(ServiceType)                   \
    service_type ServiceType::type()                        \
    {                                                       \
        static const auto id = service::type<ServiceType>();\
        return id;                                          \
    }
