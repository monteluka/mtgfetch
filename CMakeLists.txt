cmake_minimum_required(VERSION 3.28.2)
project(mtgcli)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# Function to fetch 3rd party libraries from a git repo
include(FetchContent)
function(fetch_library library_name repo_url repo_default_branch)
    if (${library_name_upper}_BRANCH_NAME STREQUAL "")
        find_program(GIT git REQUIRED)
        execute_process(COMMAND git rev-parse --abbrev-ref HEAD
                OUTPUT_VARIABLE ${library_name_upper}_BRANCH_NAME
                OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif ()
    if (${library_name_upper}_BRANCH_NAME STREQUAL HEAD)
        set(${library_name_upper}_BRANCH_NAME ${repo_default_branch})
    endif ()

    FetchContent_Declare(${library_name}
            GIT_REPOSITORY ${repo_url}
            GIT_TAG ${${library_name_upper}_BRANCH_NAME}
            GIT_SHALLOW FALSE  # ensure submodules are checked out
    )
    FetchContent_MakeAvailable(${library_name})
endfunction()

# Add cpp-httplib library for making API requests
fetch_library(httplib https://github.com/yhirose/cpp-httplib.git master)
# Add ryml library for parsing JSON & YAML
fetch_library(ryml https://github.com/biojppm/rapidyaml.git master)

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
        src/mana_symbol.cpp
        src/card_information.cpp
        src/configuration.cpp
        include/mana_symbol.h
        include/card_information.h
        include/configuration.h)

target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto httplib::httplib ryml::ryml)