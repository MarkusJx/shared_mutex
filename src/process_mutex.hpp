#ifndef SHARED_MUTEX_PROCESS_MUTEX_HPP
#define SHARED_MUTEX_PROCESS_MUTEX_HPP

#include <napi.h>
#include "shared_mutex.hpp"

/**
 * A process mutex
 */
class process_mutex : public Napi::ObjectWrap<process_mutex> {
public:
    /**
     * Initialize the class
     *
     * @param env the environment
     * @param exports the module exports
     */
    static void init(Napi::Env env, Napi::Object &exports);

    /**
     * Try creating a process_mutes
     *
     * @param info the callback info
     * @return the instance if created or null
     */
    static Napi::Value try_create(const Napi::CallbackInfo &info);

    /**
     * Create a process_mutex instance
     *
     * @param info the callback info
     */
    explicit process_mutex(const Napi::CallbackInfo &info);

    /**
     * Destroy the process mutex
     *
     * @param info the callback info
     */
    void destroy(const Napi::CallbackInfo &info);

    /**
     * Destroy the process mutex
     */
    ~process_mutex() override;

private:
    // The js constructor
    static Napi::FunctionReference *constructor;

    // The mutex instance
    std::unique_ptr<shared_mutex> instance;
};

#endif //SHARED_MUTEX_PROCESS_MUTEX_HPP
