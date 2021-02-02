#include <napi.h>
#include <map>
#include <memory>

#include "shared_mutex.hpp"

#define export(func) exports.Set(std::string("lib_") + #func, Napi::Function::New(env, func))

std::unique_ptr<std::map<std::string, shared_mutex>> mutexes = nullptr;

void createMutex(const Napi::CallbackInfo &info) {
    if (info.Length() != 1) {
        throw Napi::Error::New(info.Env(), "Expected one argument");
    } else if (!info[0].IsString()) {
        throw Napi::Error::New(info.Env(), "Expected argument type string at position 0");
    }

    try {
        mutexes->insert(
                std::pair<std::string, shared_mutex>(info[0].ToString(), std::move(shared_mutex(info[0].ToString()))));
    } catch (const std::exception &e) {
        throw Napi::Error::New(info.Env(), e.what());
    }
}

void deleteMutex(const Napi::CallbackInfo &info) {
    if (info.Length() != 1) {
        throw Napi::Error::New(info.Env(), "Expected one argument");
    } else if (!info[0].IsString()) {
        throw Napi::Error::New(info.Env(), "Expected argument type string at position 0");
    }

    try {
        mutexes->erase(info[0].ToString().Utf8Value());
    } catch (const std::exception &e) {
        throw Napi::Error::New(info.Env(), e.what());
    }
}

/**
 * Export all functions
 */
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    export(createMutex);
    export(deleteMutex);

    try {
        mutexes = std::make_unique<std::map<std::string, shared_mutex>>();
    } catch (...) {
        throw Napi::Error::New(env, "Could not create the mutex map");
    }

    return exports;
}

NODE_API_MODULE(is_program_already_running, InitAll)
