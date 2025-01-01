cmake_minimum_required(VERSION 3.16)

project(harbour-amazfish LANGUAGES CXX)

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
    add_definitions(-DKF5CALENDARCORE) ### FIME I don't know why this doesn't work with libwatchfish

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

add_subdirectory(qble)
add_subdirectory(lib)
add_subdirectory(daemon)
add_subdirectory(ui)

feature_summary(WHAT ALL)
