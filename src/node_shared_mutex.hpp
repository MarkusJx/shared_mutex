#ifndef SHARED_MUTEX_NODE_SHARED_MUTEX_HPP
#define SHARED_MUTEX_NODE_SHARED_MUTEX_HPP

#include <napi.h>
#include "shared_mutex.hpp"

/**
 * A node shared_mutex wrapper class
 */
class node_shared_mutex : public Napi::ObjectWrap<node_shared_mutex> {
public:
    /**
     * Initialize the class
     *
     * @param env the environment
     * @param exports the exports
     */
    static void init(Napi::Env env, Napi::Object &exports);

    /**
     * Create a shared_mutex wrapper
     *
     * @param info the callback info
     */
    explicit node_shared_mutex(const Napi::CallbackInfo &info);

    /**
     * Lock the mutex. Blocking call.
     *
     * @param info the callback info
     */
    void lockBlocking(const Napi::CallbackInfo &info);

    /**
     * Lock the mutex. Async call.
     *
     * @param info the callback info
     * @return the promise
     */
    Napi::Value lock(const Napi::CallbackInfo &info);

    /**
     * Try locking the mutex
     *
     * @param info the callback info
     * @return true, if the mutex could be locked
     */
    Napi::Value try_lock(const Napi::CallbackInfo &info);

    /**
     * Unlock the mutex
     *
     * @param info the callback info
     */
    void unlock(const Napi::CallbackInfo &info);

    /**
     * Destroy the mutex
     *
     * @param info the callback info
     */
    void destroy(const Napi::CallbackInfo &info);

    /**
     * Destroy the mutex
     */
    ~node_shared_mutex() override;

private:
    // The shared_mutex instance
    std::unique_ptr<shared_mutex> instance;
};

#endif //SHARED_MUTEX_NODE_SHARED_MUTEX_HPP
