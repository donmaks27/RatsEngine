#pragma once

#include <engine/core.h>

namespace engine
{
    template<typename T, typename CreateInfo>
    class service_of;

    class RATS_ENGINE_EXPORT service
    {
        template<typename T, typename CreateInfo>
        friend class service_of;

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

    template<typename T, typename CreateInfo = void>
    class service_of : public service
    {
    public:

        static_assert(std::is_class_v<CreateInfo>, "CreateInfo must be a class/struct");

        using service_create_info = CreateInfo;

        [[nodiscard]] static bool instance_create(const service_create_info& info)
        {
            if (ServiceInstance == nullptr)
            {
                const log::logger Log = T::logger();
                Log.log("Creating system...");
                service_of* instance = T::instance_allocate(info);
                if (instance == nullptr)
                {
                    Log.fatal("Failed to allocate system!");
                    return false;
                }
                ServiceInstance = instance;
                if (!instance->service_init(info))
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

        virtual bool service_init(const service_create_info& info) = 0;
    };

    template<typename T>
    class service_of<T, void> : public service
    {
    public:

        [[nodiscard]] static bool instance_create()
        {
            if (ServiceInstance == nullptr)
            {
                const log::logger Log = T::logger();
                Log.log("Creating system...");
                service_of* instance = T::instance_allocate();
                if (instance == nullptr)
                {
                    Log.fatal("Failed to allocate system!");
                    return false;
                }
                ServiceInstance = instance;
                if (!instance->service_init())
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

        virtual bool service_init() = 0;
    };
}