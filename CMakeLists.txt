cmake_minimum_required(VERSION 3.28.2)
project(mtgcli)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# Use the library with debug symbols when building in debug mode
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for Debug build")
    set(ZLIB_LIBRARY "/usr/lib64/libz.so")
    set(ZLIB_INCLUDE_DIR "/usr/include")
    set(OPENSSL_ROOT_DIR "../debugopenssl")
    set(OPENSSL_LIBRARIES "../debugopenssl/lib64")
endif ()

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp
        lib/httplib.h)

target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)