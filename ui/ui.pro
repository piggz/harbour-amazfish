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
CONFIG-=qtquickcompiler

QT +=  positioning KDb3 bluetooth quick
LIBS += -Lqble/qble -L$$OUT_PWD/../lib -lamazfish

PKGCONFIG += mlite5

INCLUDEPATH += \
    $$PWD/src/services/ \
    $$PWD/src/operations/ \
    $$PWD/src/devices/ \
    $$PWD/src/ \
    $$PWD/../ \
    ../lib/src

include(../qble/qble.pri)

SOURCES += src/harbour-amazfish-ui.cpp \
    src/datasource.cpp \
    src/sportsdatamodel.cpp \
    src/daemoninterface.cpp \
    src/sportsmetamodel.cpp \
    src/timeformatter.cpp \
    src/trackloader.cpp

DISTFILES += qml/harbour-amazfish.qml \
    qml/components/DeviceButton.qml \
    qml/components/PercentCircle.qml \
    qml/components/TruncationModes.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    qml/pages/HeartratePage.qml \
    qml/pages/PairSelectDeviceType.qml \
    qml/pics/devices/amazfit-bip.png \
    qml/pics/devices/amazfit-cor.png \
    qml/pics/devices/amazfit-gts.png \
    qml/pics/devices/miband2.png \
    qml/pics/devices/miband3.png \
    qml/pics/devices/miband4.png \
    qml/pics/devices/pinetime.png \
    qml/pics/icon-m-biking.png \
    qml/pics/icon-m-heartrate.png \
    qml/pics/icon-m-running.png \
    qml/pics/icon-m-steps.png \
    qml/pics/icon-m-treadmill.png \
    qml/pics/icon-m-walk.png \
    qml/pics/icon-m-walking.png \
    qml/pics/weather-clear-night.png \
    qml/pics/weather-clear.png \
    qml/pics/weather-clouds-night.png \
    qml/pics/weather-clouds.png \
    qml/pics/weather-few-clouds-night.png \
    qml/pics/weather-few-clouds.png \
    qml/pics/weather-freezing-rain.png \
    qml/pics/weather-hail.png \
    qml/pics/weather-many-clouds.png \
    qml/pics/weather-mist.png \
    qml/pics/weather-none-available.png \
    qml/pics/weather-showers-day.png \
    qml/pics/weather-showers-night.png \
    qml/pics/weather-showers-scattered-day.png \
    qml/pics/weather-showers-scattered-night.png \
    qml/pics/weather-showers-scattered.png \
    qml/pics/weather-showers.png \
    qml/pics/weather-snow-rain.png \
    qml/pics/weather-snow-scattered-day.png \
    qml/pics/weather-snow-scattered-night.png \
    qml/pics/weather-snow-scattered.png \
    qml/pics/weather-snow.png \
    qml/pics/weather-storm-day.png \
    qml/pics/weather-storm-night.png \
    qml/pics/weather-storm.png \
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
    qml/components/DownloadDataMenuItem.qml \
    qml/pages/AuthKeyDialog.qml \
    qml/pages/UnpairDeviceDialog.qml \
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
    src/daemoninterface.h \ \
    src/sportsdatamodel.h \
    src/sportsmetamodel.h \
    src/timeformatter.h \
    src/trackloader.h

RESOURCES += \
    amazfish.qrc
