cmake_minimum_required(VERSION 3.28.2)
project(mtgfetch
        VERSION 0.0.1
        LANGUAGES CXX
        DESCRIPTION "Magic the Gathering card information fetcher")

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

message(STATUS "PROJECT NAME: ${PROJECT_NAME}")
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
            EXCLUDE_FROM_ALL TRUE # needed so that httplib and ryml aren't a part of the project install
    )
    FetchContent_MakeAvailable(${library_name})
endfunction(fetch_library)

# Add cpp-httplib library for making API requests
fetch_library(httplib https://github.com/yhirose/cpp-httplib.git master)
# Add ryml library for parsing JSON & YAML
fetch_library(ryml https://github.com/biojppm/rapidyaml.git master)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp
        src/mana_symbol.cpp
        src/card_information.cpp
        src/configuration.cpp
        include/mana_symbol.h
        include/card_information.h
        include/configuration.h)

set(DEBUG_ASCII_PATH "${CMAKE_SOURCE_DIR}/images/ascii/")
set(RELEASE_ASCII_PATH "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}/symbols")

# Set preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASCII_ART_PATH="${DEBUG_ASCII_PATH}")
elseif (CMAKE_BUILD_TYPE STREQUAL "ReleasePackage")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASCII_ART_PATH="${RELEASE_ASCII_PATH}/ascii/")
else ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASCII_ART_PATH="/usr/local/${RELEASE_ASCII_PATH}/ascii/")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto httplib::httplib ryml::ryml)

# Installation of mtgfetch
include(GNUInstallDirs)
message(STATUS "DESTINATION FOR BIN: ${CMAKE_INSTALL_BINDIR}")
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
message(STATUS "DESTINATION FOR ASCII: ${RELEASE_ASCII_PATH}")
install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/images/ascii"
        DESTINATION "${RELEASE_ASCII_PATH}"
)