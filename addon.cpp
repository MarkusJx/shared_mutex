#include <napi.h>
#include <map>
#include <vector>
#include <memory>
#include <napi_tools.hpp>

#include "shared_mutex.hpp"

#define export(func) exports.Set(std::string("lib_") + #func, Napi::Function::New(env, func))
// Check if info has a length of 1 and info[0] is a string
#define CHECK_STRING_ARG0() if (info.Length() != 1) \
            throw Napi::Error::New(info.Env(), "Expected one argument"); \
        else if (!info[0].IsString())                  \
            throw Napi::Error::New(info.Env(), "Expected argument type string at position 0")

// The mutexes map pointer storing all created mutexes and their names
std::unique_ptr<std::map<std::string, std::unique_ptr<shared_mutex>>> mutexes = nullptr;

// The last id
long long lastId = 0;

/**
 * Ad an id to a string
 *
 * @param name the string to add the id to
 * @return the random id
 */
std::string generateRandomId(const std::string &name) {
    return name + '-' + std::to_string(lastId++);
}

/**
 * Create a shared_mutex, which throws an error
 * when a mutex with the same name already exists
 */
Napi::String createProgramMutex(const Napi::CallbackInfo &info) {
    CHECK_STRING_ARG0();

    // Try creating the mutex, rethrowing all occurring errors
    TRY
        std::string name = info[0].ToString().Utf8Value();
        std::string id = name;
        while (mutexes->find(id) != mutexes->end()) {
            id = generateRandomId(name);
        }

        mutexes->insert_or_assign(id,shared_mutex::createShared_mutex(name, false));
        return Napi::String::New(info.Env(), id);
    CATCH_EXCEPTIONS
}

/*+
 * Create a shared_mutex
 */
Napi::String createMutex(const Napi::CallbackInfo &info) {
    CHECK_STRING_ARG0();

    // Try creating the mutex, rethrowing all occurring errors
    TRY
        std::string name = info[0].ToString().Utf8Value();
        std::string id = name;
        while (mutexes->find(id) != mutexes->end()) {
            id = generateRandomId(name);
        }

        mutexes->insert_or_assign(id, shared_mutex::createShared_mutex(name, true));
        return Napi::String::New(info.Env(), id);
    CATCH_EXCEPTIONS
}

/**
 * Lock a mutex blocking
 */
void blockLock(const Napi::CallbackInfo &info) {
    CHECK_STRING_ARG0();

    TRY
        if (mutexes->find(info[0].ToString().Utf8Value()) != mutexes->end()) {
            mutexes->at(info[0].ToString().Utf8Value())->lock();
        } else {
            throw std::runtime_error("The mutex with the given name does not exist");
        }
    CATCH_EXCEPTIONS
}

/**
 * Lock a mutex while returning a promise
 */
Napi::Promise lock(const Napi::CallbackInfo &info) {
    CHECK_STRING_ARG0();

    TRY
        std::string mtx_name = info[0].ToString().Utf8Value();

        return napi_tools::promises::promise<void>(info.Env(), [mtx_name] {
            if (mutexes->find(mtx_name) != mutexes->end()) {
                mutexes->at(mtx_name)->lock();
            } else {
                throw std::runtime_error("The mutex with the given name does not exist");
            }
        });
    CATCH_EXCEPTIONS
}

/**
 * Try locking a mutex
 */
Napi::Boolean try_lock(const Napi::CallbackInfo &info) {
    CHECK_STRING_ARG0();

    TRY
        if (mutexes->find(info[0].ToString().Utf8Value()) != mutexes->end()) {
            return Napi::Boolean::New(info.Env(), mutexes->at(info[0].ToString().Utf8Value())->try_lock());
        } else {
            throw std::runtime_error("The mutex with the given name does not exist");
        }
    CATCH_EXCEPTIONS
}

/**
 * Unlock mutex
 */
void unlock(const Napi::CallbackInfo &info) {
    CHECK_STRING_ARG0();

    TRY
        if (mutexes->find(info[0].ToString().Utf8Value()) != mutexes->end()) {
            mutexes->at(info[0].ToString().Utf8Value())->unlock();
        } else {
            throw std::runtime_error("The mutex with the given name does not exist");
        }
    CATCH_EXCEPTIONS
}

/**
 * Delete a mutex
 */
void deleteMutex(const Napi::CallbackInfo &info) {
    CHECK_STRING_ARG0();

    // Try erasing the mutex, rethrowing all occurring errors
    TRY
        if (mutexes->find(info[0].ToString().Utf8Value()) != mutexes->end()) {
            mutexes->erase(info[0].ToString().Utf8Value());
        } else {
            throw std::runtime_error("The mutex with the given name does not exist");
        }
    CATCH_EXCEPTIONS
}

/**
 * Export all functions
 */
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    // Export the functions
    export(createProgramMutex);
    export(createMutex);
    export(blockLock);
    export(lock);
    export(try_lock);
    export(unlock);
    export(deleteMutex);

    // Try creating the mutexes map pointer
    try {
        mutexes = std::make_unique<std::map<std::string, std::unique_ptr<shared_mutex>>>();
    } catch (...) {
        throw Napi::Error::New(env, "Could not create the mutex map");
    }

    return exports;
}

NODE_API_MODULE(shared_mutex, InitAll)
