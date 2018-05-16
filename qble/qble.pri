TEMPLATE = lib

qt += dbus

HEADERS += \
    $$PWD/qbledevice.h \
    $$PWD/qbleservice.h \
    $$PWD/qblecharacteristic.h

SOURCES += \
    $$PWD/qbledevice.cpp \
    $$PWD/qbleservice.cpp \
    $$PWD/qblecharacteristic.cpp

