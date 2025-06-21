cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(libamazfish STATIC)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Xml)

if(FLAVOR STREQUAL "silica")

    pkg_check_modules(MLITE5 REQUIRED mlite5)

endif()


target_include_directories(libamazfish PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${MLITE5_INCLUDE_DIRS})
target_link_libraries(libamazfish PUBLIC Qt5::Core Qt5::DBus Qt5::Xml ${MLITE5_LIBRARIES})

set(SOURCES
    ./src/amazfishconfig.cpp
    ./src/weather/city.cpp
    ./src/weather/citymanager.cpp
    ./src/weather/citysearchmodel.cpp
    ./src/weather/currentweather.cpp
    ./src/activitykind.cpp
)

set(HEADERS
    ./src/weather/apikey.h
    ./src/weather/city.h
    ./src/weather/citymanager.h
    ./src/weather/citysearchmodel.h
    ./src/weather/currentweather.h
    ./src/amazfish.h
    ./src/amazfishconfig.h
    ./src/activitykind.h
)

target_sources(libamazfish PUBLIC ${SOURCES} ${HEADERS})

