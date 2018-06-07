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
TEMPLATE = app

CONFIG += sailfishapp
LIBS += -Lqble/qble

QT +=  contacts KDb3

PKGCONFIG += mlite5

SOURCES += src/harbour-amazfish.cpp \
    src/qaesencryption.cpp \
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
    src/bipdevice.cpp \
    src/deviceinterface.cpp \
    src/deviceinfoservice.cpp \
    src/activitysample.cpp \
    src/datasource.cpp \
    src/operations/abstractoperation.cpp \
    src/operations/activityfetchoperation.cpp \
    src/operations/logfetchoperation.cpp \
    src/operations/sportssummaryoperation.cpp
 
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
    qml/pages/Settings-device.qml \
    qml/pages/Settings-app.qml \
    qml/pages/AnalysisPage.qml \
    qml/pages/Axis.qml \
    qml/pages/GraphData.qml \
    qml/components/Axis.qml \
    qml/components/Graph.qml \
    qml/components/GraphData.qml \
    qml/pages/Settings-bip-shortcuts.qml \
    qml/components/Alarm.qml \
    qml/pages/Settings-alarms.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-amazfish-de.ts \
                translations/harbour-amazfish-ru.ts \
                translations/harbour-amazfish-sv.ts

HEADERS += \
    src/qaesencryption.h \
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
    src/bipdevice.h \
    src/deviceinterface.h \
    src/deviceinfoservice.h \
    src/activitysample.h \
    src/datasource.h \
    src/operations/abstractoperation.h \
    src/operations/activityfetchoperation.h \
    src/operations/logfetchoperation.h \
    src/operations/sportssummaryoperation.h

include(qble/qble.pri)
