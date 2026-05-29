cmake_minimum_required(VERSION 3.16)

project(harbour-amazfish LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(FeatureSummary)

find_package(Qt5 COMPONENTS Core Gui Quick Sql DBus LinguistTools REQUIRED)

if(NOT DEFINED FLAVOR)
    message(FATAL_ERROR "Please specify the platform using -DFLAVOR=platform as a CMake option. Supported platforms: kirigami, silica, qtcontrols, uuitk.")
elseif(FLAVOR STREQUAL "silica")
    add_definitions(-DMER_EDITION_SAILFISH)
    add_definitions(-DKF5CALENDARCORE) ### FIXME this should be in libwatchfish

    include(FindPkgConfig)
    pkg_search_module(SAILFISH sailfishapp REQUIRED)
elseif(FLAVOR STREQUAL "kirigami")
    add_definitions(-Dflavor_kirigami)
elseif(FLAVOR STREQUAL "qtcontrols")
    add_definitions(-Dflavor_qtcontrols)
elseif(FLAVOR STREQUAL "uuitk")
    add_definitions(-DUUITK_EDITION)
    add_subdirectory(click)
else()
    message(FATAL_ERROR "Unsupported platform '${FLAVOR}'. Supported platforms: kirigami, silica, qtcontrols, uuitk.")
endif()

# Allow manual override via cmake -DPROJECT_GIT_VERSION=v2.9.0
set(PROJECT_GIT_VERSION "" CACHE STRING "Override git version string")

if(PROJECT_GIT_VERSION STREQUAL "")
    find_program(GIT_EXECUTABLE git)
    if(GIT_EXECUTABLE)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --always --long --dirty
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE PROJECT_GIT_VERSION
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT GIT_DESCRIBE_RESULT EQUAL 0)
            set(PROJECT_GIT_VERSION "unknown")
        endif()
    else()
        set(PROJECT_GIT_VERSION "unknown")
    endif()
endif()

message(STATUS "Git version: ${PROJECT_GIT_VERSION}")
add_compile_definitions(PROJECT_GIT_VERSION="${PROJECT_GIT_VERSION}")

add_subdirectory(qble)
add_subdirectory(lib)
add_subdirectory(daemon)
add_subdirectory(ui)

feature_summary(WHAT ALL)
