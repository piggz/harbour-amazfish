TARGET = amazfish

TEMPLATE = lib

CONFIG += staticlib
CONFIG += link_pkgconfig

QT += core network

equals(FLAVOR, "silica") {
    CONFIG += flavor_silica
} else:equals(FLAVOR, "kirigami") {
    CONFIG += flavor_kirigami
} else:equals(FLAVOR, "qtcontrols") {
    CONFIG += flavor_qtcontrols
} else:equals(FLAVOR, "uuitk") {
    CONFIG += flavor_uuitk
} else {
    error("Please specify platform using FLAVOR=platform as qmake option. Supported platforms: kirigami, silica, qtcontrols, uuitk.")
}

flavor_silica {
    message(SailfishOS lib build)
    DEFINES += MER_EDITION_SAILFISH
    PKGCONFIG += mlite5
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
