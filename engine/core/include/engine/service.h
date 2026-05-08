#pragma once

#include <engine/core.h>

#include <engine/render/render_api.h>
#include <engine/utils/type_storage.h>

namespace engine
{
    using service_type = std::uint8_t;
    constexpr service_type invalid_service_type = utils::type_storage<service_type>::invalid_id;

    class engine;
    template<typename T>
    class service_impl;

    class RATS_ENGINE_EXPORT service
    {
        friend engine;
        template<typename T>
        friend class service_impl;

    protected:
        service() = default;
        virtual ~service() = default;
    public:
        service(const service&) = delete;
        service(service&&) = delete;

        service& operator=(const service&) = delete;
        service& operator=(service&&) = delete;

        [[nodiscard]] static constexpr auto logger() { return log::logger("service"); }

    protected:

        [[nodiscard]] virtual bool service_init() = 0;
        virtual void service_clear() = 0;

    private:

        static utils::type_storage<service_type> ServiceTypes;
    };

    template<typename T>
    class service_impl : public service
    {
    public:
        [[nodiscard]] static service_type type()
        {
            static const service_type id = ServiceTypes.type_id<T>();
            return id;
        }
    };

    template<typename T>
    class service_of : public service_impl<T>
    {
    public:
        using this_t = service_of;
    };
}

#define RATS_ENGINE_SERVICE(Type, LogCategory)                                                              \
    public:                                                                                                 \
        using super_t = this_t;                                                                             \
        using this_t = Type;                                                                                \
        Type() { Instance = this; }                                                                         \
        virtual ~Type() override { Instance = nullptr; }                                                    \
    public:                                                                                                 \
        static const log::logger Log;                                                                       \
        [[nodiscard]] static constexpr log::logger logger() { return { LogCategory, super_t::logger() }; }  \
        [[nodiscard]] static auto& instance() { return *Instance; }                                         \
    private:                                                                                                \
        static Type* Instance;

#define RATS_ENGINE_SERVICE_BASE(Type, LogCategory) \
    RATS_ENGINE_SERVICE(Type, LogCategory)          \
    public:                                         \
        friend super_t;                             \
    private:

#define RATS_ENGINE_SERVICE_IMPL(Type)                      \
    Type* Type::Instance = nullptr;                         \
    const ::engine::log::logger Type::Log = Type::logger();
