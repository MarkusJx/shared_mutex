cmake_minimum_required(VERSION 3.15)
project(shared_mutex)

execute_process(COMMAND node -p "require('@markusjx/n-api-tools').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE N_API_TOOLS_DIR)

add_compile_definitions(NAPI_CPP_EXCEPTIONS)
include_directories(${CMAKE_JS_INC} ${N_API_TOOLS_DIR})

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} SHARED src/addon.cpp src/shared_mutex.hpp ${CMAKE_JS_SRC} src/node_shared_mutex.cpp
        src/node_shared_mutex.hpp src/process_mutex.cpp src/process_mutex.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

# define NAPI_VERSION
add_definitions(-DNAPI_VERSION=6)