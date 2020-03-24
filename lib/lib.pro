TARGET = amazfish

TEMPLATE = lib

CONFIG += staticlib

QT += core

PKGCONFIG += mlite5

INCLUDEPATH += /usr/include/mlite5

HEADERS = \
    src/amazfishconfig.h

SOURCES = \
    src/amazfishconfig.cpp
