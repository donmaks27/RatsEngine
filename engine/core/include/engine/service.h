#pragma once

#include <engine/core.h>

#include <engine/utils/type_storage.h>

#include <functional>

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

    namespace registration
    {
        class service_storage;

        class service final
        {
            friend core_engine;
            friend service_storage;

        public:
            service() = delete;
            service(service_type type);
            service(const service&) = delete;
            service(service&&) = delete;
            ~service() = default;

            service& operator=(const service&) = delete;
            service& operator=(service&&) = delete;

            template<typename Func> requires std::predicate<Func> && base_service_class<std::remove_pointer_t<std::invoke_result_t<Func>>>
            service& allocator(Func&& func)
            {
                if (m_type == std::remove_pointer_t<std::invoke_result_t<Func>>::type())
                {
                    m_allocator = [f = std::forward<Func>(func)]() -> engine::service* { return f(); };
                }
                return *this;
            }

            template<typename T> requires base_service_class<T>
            service& init_before()
            {
                if (m_type != T::type())
                {
                    m_initBefore.push_back(T::type());
                }
                return *this;
            }
            template<typename T> requires base_service_class<T>
            service& init_after()
            {
                if (m_type != T::type())
                {
                    m_initAfter.push_back(T::type());
                }
                return *this;
            }

        private:

            std::function<engine::service*()> m_allocator = nullptr;
            eastl::vector<service_type> m_initBefore;
            eastl::vector<service_type> m_initAfter;

            service_type m_type = invalid_service_type;
        };

        class service_storage final
        {
            friend core_engine;

            service_storage() = default;
            ~service_storage() = default;
        public:
            service_storage(const service_storage&) = delete;
            service_storage(service_storage&&) = delete;

            service_storage& operator=(const service_storage&) = delete;
            service_storage& operator=(service_storage&&) = delete;

            template<typename T> requires base_service_class<T>
            [[nodiscard]] service& register_service() { return register_service(T::type()); }

        private:

            eastl::unordered_map<service_type, service> m_services;

            [[nodiscard]] service& register_service(service_type type);
        };
    }
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
