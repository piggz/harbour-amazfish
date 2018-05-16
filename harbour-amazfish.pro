# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-amazfish

CONFIG += sailfishapp

include(qble/qble.pri)

QT += bluetooth dbus contacts

PKGCONFIG += mlite5

SOURCES += src/harbour-amazfish.cpp \
    src/bipinterface.cpp \
    src/qaesencryption.cpp \
    src/bippair.cpp \
    src/bipinfoservice.cpp \
    src/mibandservice.cpp \
    src/miband2service.cpp \
    src/alertnotificationservice.cpp \
    src/notificationslistener.cpp \
    src/voicecallhandler.cpp \
    src/voicecallmanager.cpp \
    src/typeconversion.cpp \
    src/settingsmanager.cpp \
    src/bipbatteryinfo.cpp \
    src/hrmservice.cpp \
    src/bipdevice.cpp

DISTFILES += qml/harbour-amazfish.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    rpm/harbour-amazfish.changes.in \
    rpm/harbour-amazfish.changes.run.in \
    rpm/harbour-amazfish.spec \
    rpm/harbour-amazfish.yaml \
    translations/*.ts \
    harbour-amazfish.desktop \
    qml/pages/PairPage.qml \
    qml/pages/DebugInfo.qml \
    qml/pages/Settings-profile.qml \
    qml/pages/Settings-menu.qml \
    qml/pages/Settings-device.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-amazfish-de.ts

HEADERS += \
    src/bipinterface.h \
    src/bipinterface.h \
    src/qaesencryption.h \
    src/bippair.h \
    src/bipinfoservice.h \
    src/mibandservice.h \
    src/miband2service.h \
    src/alertnotificationservice.h \
    src/notificationslistener.h \
    src/voicecallhandler.h \
    src/voicecallmanager.h \
    src/typeconversion.h \
    src/settingsmanager.h \
    src/bipbatteryinfo.h \
    src/hrmservice.h \
    src/bipdevice.h
