#pragma once

#include <engine/core.h>

#include <engine/engine.h>
#include <engine/render/render_api.h>
#include <engine/utils/type_storage.h>

#include <EASTL/array.h>

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

        virtual void service_clear() = 0;

    private:

        static utils::type_storage<service_type> ServiceTypes;
        static eastl::array<service*, std::numeric_limits<service_type>::max()> ServiceInstances;
        static bool ServiceAllocateEnabled;
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

        [[nodiscard]] static bool instance_create()
        {
            if (!ServiceAllocateEnabled)
            {
                return false;
            }
            auto& serviceInstance = ServiceInstances[type()];
            if (serviceInstance == nullptr)
            {
                T::Log.log("Creating service...");
                service_impl* instance = T::instance_allocate_impl();
                if (instance == nullptr)
                {
                    T::Log.fatal("Failed to allocate service!");
                    return false;
                }
                serviceInstance = instance;
                if (!instance->service_init())
                {
                    T::Log.fatal("Failed to initialize service!");
                    serviceInstance->service_clear();
                    delete serviceInstance;
                    serviceInstance = nullptr;
                    return false;
                }
                T::Log.info("Service created successfully");
            }
            return true;
        }
        static void instance_clear()
        {
            auto& serviceInstance = ServiceInstances[type()];
            if (serviceInstance != nullptr)
            {
                T::Log.log("Clearing service...");
                serviceInstance->service_clear();
                delete serviceInstance;
                serviceInstance = nullptr;
                T::Log.log("Service cleared successfully");
            }
        }

    protected:

        virtual bool service_init() = 0;
    };

    template<typename T>
    class service_of : public service_impl<T>
    {
        friend service_impl<T>;

    public:
        using this_t = service_of;
    private:

        [[nodiscard]] static T* instance_allocate_impl()
        {
            T* result = nullptr;
            const auto& cfg = engine::instance().config();
            switch (cfg.renderApi)
            {
                case render_api::vulkan: result = T::instance_allocate_vulkan(); break;
                default:;
            }
            if (result == nullptr)
            {
                T::Log.fatal("Render API '{}' is not implemented", cfg.renderApi);
            }
            return result;
        }
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
