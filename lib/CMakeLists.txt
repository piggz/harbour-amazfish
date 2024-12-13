cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(libamazfish STATIC)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Xml)

target_include_directories(libamazfish PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(libamazfish PUBLIC Qt5::Core Qt5::DBus Qt5::Xml)

set(SOURCES
    ./src/amazfish.cpp
    ./src/amazfishconfig.cpp
    ./src/weather/city.cpp
    ./src/weather/citymanager.cpp
    ./src/weather/citysearchmodel.cpp
    ./src/weather/currentweather.cpp
)

set(HEADERS
    ./src/weather/apikey.h
    ./src/weather/city.h
    ./src/weather/citymanager.h
    ./src/weather/citysearchmodel.h
    ./src/weather/currentweather.h
    ./src/amazfish.h
    ./src/amazfishconfig.h
)

target_sources(libamazfish PUBLIC ${SOURCES} ${HEADERS})

