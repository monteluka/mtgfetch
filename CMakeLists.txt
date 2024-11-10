cmake_minimum_required(VERSION 3.28.2)
project(mtgfetch
        VERSION 0.0.1
        LANGUAGES CXX
        DESCRIPTION "Magic the Gathering card information fetcher")

set(CMAKE_CXX_STANDARD 17)

if(NOT LINUX)
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}.\nCurrently mtgfetch only works on linux.")
endif ()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "ReleasePackage")
    set(CMAKE_INSTALL_PREFIX /usr)
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
    target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_DIR_PATH="${CMAKE_SOURCE_DIR}/presets")
elseif (CMAKE_BUILD_TYPE STREQUAL "ReleasePackage")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASCII_ART_PATH="/usr/${RELEASE_ASCII_PATH}/ascii/")
    target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_DIR_PATH="/usr/${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}/presets")
else ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE ASCII_ART_PATH="/usr/local/${RELEASE_ASCII_PATH}/ascii/")
    target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_DIR_PATH="/usr/local/${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}/presets")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto httplib::httplib ryml::ryml)

# Installation of mtgfetch
include(GNUInstallDirs)
# install the binary
message(STATUS "DESTINATION FOR BIN: ${CMAKE_INSTALL_BINDIR}")
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
# install the ascii txt files that are used for mana symbols
message(STATUS "DESTINATION FOR ASCII: ${RELEASE_ASCII_PATH}")
install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/images/ascii"
        DESTINATION "${RELEASE_ASCII_PATH}"
)
# install the preset configurations
install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/presets"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}"
)
# install bash completions
install(
        FILES "${CMAKE_SOURCE_DIR}/completions/${CMAKE_PROJECT_NAME}.bash"
        DESTINATION "/etc/bash_completion.d"
        RENAME "${CMAKE_PROJECT_NAME}"
)

# package mtgfetch
set(CPACK_PACKAGE_CONTACT "Monte <dmonterrosa@pm.me>")
set(CPACK_PACKAGE_DESCRIPTION "mtgfetch is a tool to fetch information on a single Magic: The Gathering card and display it to the terminal")
set(CPACK_GENERATOR "TGZ;")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_HOST_SYSTEM_PROCESSOR}")
find_program(HAVE_RPMBUILD "rpmbuild")
if (HAVE_RPMBUILD)
    set(CPACK_GENERATOR "${CPACK_GENERATOR};RPM")
    set(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
endif ()
find_program(HAVE_DPKG "dpkg")
if (HAVE_DPKG)
    set(CPACK_GENERATOR "${CPACK_GENERATOR};DEB")
    set(CPACK_DEBIAN_PACKAGE_SECTION, "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
endif ()

include(CPack)