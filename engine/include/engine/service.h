#pragma once

#include <engine/core.h>

namespace engine
{
    using service_type = std::uint8_t;
    constexpr service_type invalid_service_type = std::numeric_limits<service_type>::max();

    template<typename T, typename... CreateInfo>
    class service_impl;

    class RATS_ENGINE_EXPORT service
    {
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

        static service* ServiceInstance;

        virtual void service_clear() = 0;
    };

    template<typename T, typename... CreateArgs>
    class service_impl : public service
    {
    public:

        [[nodiscard]] static bool instance_create(CreateArgs&&... args)
        {
            if (ServiceInstance == nullptr)
            {
                const log::logger Log = T::logger();
                Log.log("Creating system...");
                service_impl* instance = T::instance_allocate(args...);
                if (instance == nullptr)
                {
                    Log.fatal("Failed to allocate system!");
                    return false;
                }
                ServiceInstance = instance;
                if (!instance->service_init(args...))
                {
                    Log.fatal("Failed to initialize system!");
                    ServiceInstance->service_clear();
                    delete ServiceInstance;
                    ServiceInstance = nullptr;
                    return false;
                }
                Log.info("System created successfully");
            }
            return true;
        }
        static void instance_clear()
        {
            if (ServiceInstance != nullptr)
            {
                const log::logger Log = T::logger();
                Log.log("Clearing system...");
                ServiceInstance->service_clear();
                delete ServiceInstance;
                ServiceInstance = nullptr;
                Log.log("System cleared successfully");
            }
        }

    protected:

        virtual bool service_init(CreateArgs&&... info) = 0;
    };

    template<typename T, typename CreateInfo>
    class service_of : public service_impl<T, const CreateInfo&>
    {
    public:
        static_assert(std::is_class_v<CreateInfo>, "CreateInfo must be a class/struct");
        using service_create_info = CreateInfo;
    };
    template<typename T>
    class service_of<T, void> : public service_impl<T>
    {
    };
}