cmake_minimum_required(VERSION 3.28.2)
project(mtgcli)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# Add ryml library for parsing JSON & YAML
if (RYML_BRANCH_NAME STREQUAL "")
    find_program(GIT git REQUIRED)
    execute_process(COMMAND git rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE RYML_BRANCH_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()
if (RYML_BRANCH_NAME STREQUAL HEAD)
    set(RYML_BRANCH_NAME master)
endif ()
message(STATUS "FetchContent from branch: ${RYML_BRANCH_NAME}")

include(FetchContent)
FetchContent_Declare(ryml
        GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
        GIT_TAG ${RYML_BRANCH_NAME}
        GIT_SHALLOW FALSE  # ensure submodules are checked out
)
FetchContent_MakeAvailable(ryml)

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
        lib/httplib.h
        src/mana_symbol.cpp
        src/card_information.cpp
        src/configuration.cpp
        include/mana_symbol.h
        include/card_information.h
        include/configuration.h)

target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto ryml::ryml)