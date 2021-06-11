#include <napi.h>
#include <vector>

#include "node_shared_mutex.hpp"
#include "process_mutex.hpp"

/**
 * Export all functions
 */
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    // Export the functions
    node_shared_mutex::init(env, exports);
    process_mutex::init(env, exports);

    return exports;
}

NODE_API_MODULE(shared_mutex, InitAll)
