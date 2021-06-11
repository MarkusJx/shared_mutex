#include "process_mutex.hpp"
#include <napi_tools.hpp>

#define CHECK_CREATED() if (!instance) throw Napi::Error::New(info.Env(), "The mutex is not initialized")

void process_mutex::init(Napi::Env env, Napi::Object &exports) {
    Napi::Function func = DefineClass(env, "process_mutex", {
            StaticMethod("try_create", &process_mutex::try_create, napi_enumerable),
            InstanceMethod("destroy", &process_mutex::destroy, napi_enumerable)
    });

    constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);

    exports.Set("process_mutex", func);
    env.SetInstanceData<Napi::FunctionReference>(constructor);
}

Napi::Value process_mutex::try_create(const Napi::CallbackInfo &info) {
    CHECK_ARGS(napi_tools::string);

    if (constructor == nullptr) {
        return info.Env().Null();
    }

    try {
        return constructor->New({info[0].ToString()});
    } catch (...) {
        return info.Env().Null();
    }
}

process_mutex::process_mutex(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    CHECK_ARGS(napi_tools::string);
    const std::string name = info[0].ToString().Utf8Value();

    TRY
        instance = shared_mutex::createShared_mutex(name, false);
    CATCH_EXCEPTIONS
}

void process_mutex::destroy(const Napi::CallbackInfo &info) {
    CHECK_CREATED();

    TRY
        instance.reset();
    CATCH_EXCEPTIONS
}

process_mutex::~process_mutex() = default;

Napi::FunctionReference *process_mutex::constructor = nullptr;
