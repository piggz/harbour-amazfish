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
TARGET = harbour-amazfish-ui

CONFIG += sailfishapp

QT +=  contacts positioning KDb3
LIBS += -Lqble/qble

PKGCONFIG += mlite5

INCLUDEPATH += $$PWD/src/services/ \
                                    $$PWD/src/operations/ \
                                    $$PWD/src/devices/ \
                                    $$PWD/src/ \
                                    $$PWD/../

include(../qble/qble.pri)

SOURCES += src/harbour-amazfish-ui.cpp \
    src/datasource.cpp \
    src/sportsdatamodel.cpp \
    src/weather/citysearchmodel.cpp \
    src/weather/city.cpp \
    src/weather/citymanager.cpp \
    src/daemoninterface.cpp

DISTFILES += qml/harbour-amazfish.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    rpm/harbour-amazfish.changes.in \
    rpm/harbour-amazfish.changes.run.in \
    rpm/harbour-amazfish.spec \
    rpm/harbour-amazfish.yaml \
    translations/*.ts \
    qml/pages/PairPage.qml \
    qml/pages/DebugInfo.qml \
    qml/pages/Settings-profile.qml \
    qml/pages/Settings-menu.qml \
    qml/pages/Settings-device.qml \
    qml/pages/Settings-app.qml \
    qml/pages/Axis.qml \
    qml/pages/GraphData.qml \
    qml/components/Axis.qml \
    qml/components/Graph.qml \
    qml/components/GraphData.qml \
    qml/pages/Settings-bip-shortcuts.qml \
    qml/components/Alarm.qml \
    qml/pages/Settings-alarms.qml \
    qml/pages/BipFirmwarePage.qml \
    qml/pages/SportsSummaryPage.qml \
    qml/pages/AddCityDialog.qml \
    qml/pages/AnalysisPage.qml \
    qml/pages/StepsPage.qml \
    qml/pages/SleepPage.qml \
    harbour-amazfish-ui.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-amazfish-ui-de.ts \
                translations/harbour-amazfish-ui-ru.ts \
                translations/harbour-amazfish-ui-sv.ts \
                translations/harbour-amazfish-ui-es.ts
HEADERS += \
    src/datasource.h \
    src/sportsdatamodel.h \
    src/weather/city.h \
    src/weather/citymanager.h \
    src/weather/citysearchmodel.h \
    src/weather/huamiweathercondition.h \
    src/daemoninterface.h

