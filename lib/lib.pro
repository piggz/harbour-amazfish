TARGET = amazfish

TEMPLATE = lib

CONFIG += staticlib

QT += core

exists("/usr/lib/qt5/qml/Sailfish/Silica/SilicaGridView.qml"): {
    message(SailfishOS lib build)
    DEFINES += MER_EDITION_SAILFISH
    PKGCONFIG += mlite5
    INCLUDEPATH += /usr/include/mlite5
}

HEADERS = \
    src/amazfishconfig.h \
    src/weather/apikey.h \
    src/weather/city.h \
    src/weather/citymanager.h \
    src/weather/citysearchmodel.h \
    src/weather/currentweather.h

SOURCES = \
    src/amazfishconfig.cpp \
    src/weather/city.cpp \
    src/weather/citymanager.cpp \
    src/weather/citysearchmodel.cpp \
    src/weather/currentweather.cpp
