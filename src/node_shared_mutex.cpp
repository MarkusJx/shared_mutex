#include "node_shared_mutex.hpp"
#include <napi_tools.hpp>

#define CHECK_CREATED() if (!instance) throw Napi::Error::New(info.Env(), "The mutex is not initialized")

void node_shared_mutex::init(Napi::Env env, Napi::Object &exports) {
    Napi::Function func = DefineClass(env, "shared_mutex", {
            InstanceMethod("lock_blocking", &node_shared_mutex::lockBlocking, napi_enumerable),
            InstanceMethod("lock", &node_shared_mutex::lock, napi_enumerable),
            InstanceMethod("try_lock", &node_shared_mutex::try_lock, napi_enumerable),
            InstanceMethod("unlock", &node_shared_mutex::unlock, napi_enumerable),
            InstanceMethod("destroy", &node_shared_mutex::destroy, napi_enumerable)
    });

    auto *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);

    exports.Set("shared_mutex", func);
    env.SetInstanceData<Napi::FunctionReference>(constructor);
}

node_shared_mutex::node_shared_mutex(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    CHECK_ARGS(napi_tools::string);
    const std::string name = info[0].ToString().Utf8Value();

    TRY
        instance = shared_mutex::createShared_mutex(name, true);
    CATCH_EXCEPTIONS
}

void node_shared_mutex::lockBlocking(const Napi::CallbackInfo &info) {
    CHECK_CREATED();

    TRY
        instance->lock();
    CATCH_EXCEPTIONS
}

Napi::Value node_shared_mutex::lock(const Napi::CallbackInfo &info) {
    return napi_tools::promises::promise<void>(info.Env(), [this] {
        if (!instance) {
            throw std::runtime_error("The mutex is not initialized");
        }

        instance->lock();
    });
}

Napi::Value node_shared_mutex::try_lock(const Napi::CallbackInfo &info) {
    CHECK_CREATED();

    TRY
        return Napi::Boolean::New(info.Env(), instance->try_lock());
    CATCH_EXCEPTIONS
}

void node_shared_mutex::unlock(const Napi::CallbackInfo &info) {
    CHECK_CREATED();

    TRY
        instance->unlock();
    CATCH_EXCEPTIONS
}

void node_shared_mutex::destroy(const Napi::CallbackInfo &info) {
    CHECK_CREATED();

    TRY
        instance.reset();
    CATCH_EXCEPTIONS
}

node_shared_mutex::~node_shared_mutex() = default;