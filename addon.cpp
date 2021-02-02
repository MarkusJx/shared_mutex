#include <napi.h>
#include <map>
#include <memory>

#include "shared_mutex.hpp"

#define export(func) exports.Set(std::string("lib_") + #func, Napi::Function::New(env, func))

// The mutexes map pointer storing all created mutexes and their names
std::unique_ptr<std::map<std::string, shared_mutex>> mutexes = nullptr;

/**
 * Create a shared_mutex
 */
void createMutex(const Napi::CallbackInfo &info) {
    // Check if info has a length of 1 and info[0] is a string
    if (info.Length() != 1) {
        throw Napi::Error::New(info.Env(), "Expected one argument");
    } else if (!info[0].IsString()) {
        throw Napi::Error::New(info.Env(), "Expected argument type string at position 0");
    }

    // Try creating the mutex, rethrowing all occurring errors
    try {
        mutexes->insert(
                std::pair<std::string, shared_mutex>(info[0].ToString(), std::move(shared_mutex(info[0].ToString()))));
    } catch (const std::exception &e) {
        throw Napi::Error::New(info.Env(), e.what());
    }
}

void deleteMutex(const Napi::CallbackInfo &info) {
    // Check if info has a length of 1 and info[0] is a string
    if (info.Length() != 1) {
        throw Napi::Error::New(info.Env(), "Expected one argument");
    } else if (!info[0].IsString()) {
        throw Napi::Error::New(info.Env(), "Expected argument type string at position 0");
    }

    // Try erasing the mutex, rethrowing all occurring errors
    try {
        if (mutexes->find(info[0].ToString().Utf8Value()) != mutexes->end()) {
            mutexes->erase(info[0].ToString().Utf8Value());
        } else {
            throw std::runtime_error("The mutex with the given name does not exist");
        }
    } catch (const std::exception &e) {
        throw Napi::Error::New(info.Env(), e.what());
    }
}

/**
 * Export all functions
 */
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    // Export the functions
    export(createMutex);
    export(deleteMutex);

    // Try creating the mutexes map pointer
    try {
        mutexes = std::make_unique<std::map<std::string, shared_mutex>>();
    } catch (...) {
        throw Napi::Error::New(env, "Could not create the mutex map");
    }

    return exports;
}

NODE_API_MODULE(is_program_already_running, InitAll)
