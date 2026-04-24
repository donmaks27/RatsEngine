#pragma once

#include <engine/core.h>

#include <engine/utils/type_storage.h>
#include <engine/render/types.h>

#include <EASTL/array.h>

namespace engine
{
    using service_type = std::uint8_t;
    constexpr service_type invalid_service_type = utils::type_storage<service_type>::invalid_id;

    class engine;
    template<typename T, typename... CreateInfo>
    class service_impl;

    class RATS_ENGINE_EXPORT service
    {
        friend engine;
        template<typename T, typename... CreateInfo>
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

    template<typename T, typename... CreateArgs>
    class service_impl : public service
    {
    public:
        [[nodiscard]] static service_type type()
        {
            static const service_type id = ServiceTypes.type_id<T>();
            return id;
        }

        [[nodiscard]] static bool instance_create(CreateArgs&&... args)
        {
            if (!ServiceAllocateEnabled)
            {
                return false;
            }
            auto& serviceInstance = ServiceInstances[type()];
            if (serviceInstance == nullptr)
            {
                T::Log.log("Creating service...");
                service_impl* instance = T::instance_allocate_impl(args...);
                if (instance == nullptr)
                {
                    T::Log.fatal("Failed to allocate service!");
                    return false;
                }
                serviceInstance = instance;
                if (!instance->service_init(args...))
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

        virtual bool service_init(CreateArgs&&... info) = 0;
    };

    template<typename T, typename CreateInfo>
    class service_of : public service_impl<T, const CreateInfo&>
    {
        friend service_impl<T, const CreateInfo&>;

    public:
        static_assert(std::is_class_v<CreateInfo>, "CreateInfo must be a class/struct");
        using service_create_info = CreateInfo;

    private:

        [[nodiscard]] static T* instance_allocate_impl(const service_create_info& info)
        {
            return T::instance_allocate(info);
        }
    };
    template<typename T>
    class service_of<T, void> : public service_impl<T>
    {
        friend service_impl<T>;

        [[nodiscard]] static T* instance_allocate_impl()
        {
            return T::instance_allocate();
        }
    };

    template<typename CreateInfo>
    concept render_api_class = requires(CreateInfo info) {
        { info.renderApi } -> std::convertible_to<render_api>;
    };
    struct render_api_service_create_info
    {
        render_api renderApi = render_api::vulkan;
    };

    template<typename T, render_api_class CreateInfo>
    class service_of<T, CreateInfo> : public service_impl<T, const CreateInfo&>
    {
        friend service_impl<T, const CreateInfo&>;

    public:
        using service_create_info_t = CreateInfo;
        using this_t = service_of;
    private:

        [[nodiscard]] static T* instance_allocate_impl(const service_create_info_t& info)
        {
            T* result = nullptr;
            switch (info.renderApi)
            {
                case render_api::vulkan: result = T::instance_allocate_vulkan(); break;
                default:;
            }
            if (result == nullptr)
            {
                T::Log.fatal("Render API '{}' is not implemented", info.renderApi);
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
        [[nodiscard]] static auto instance() { return Instance; }                                           \
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
