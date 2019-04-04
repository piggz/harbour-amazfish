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
LIBS += -Lqble/qble

QT +=  contacts positioning KDb3

PKGCONFIG += mlite5

INCLUDEPATH += $$PWD/src/services/ \
                                    $$PWD/src/operations/ \
                                    $$PWD/src/devices/ \
                                    $$PWD/src/

include(qble/qble.pri)

SOURCES += src/harbour-amazfish.cpp \
    src/qaesencryption.cpp \
    src/voicecallhandler.cpp \
    src/voicecallmanager.cpp \
    src/typeconversion.cpp \
    src/settingsmanager.cpp \
    src/bipbatteryinfo.cpp \
    src/datasource.cpp \
    src/notificationslistener.cpp \
    src/devicefactory.cpp \
    src/services/mibandservice.cpp \
    src/services/miband2service.cpp \
    src/services/alertnotificationservice.cpp \
    src/services/hrmservice.cpp \
    src/services/deviceinfoservice.cpp \
    src/services/bipfirmwareservice.cpp \
    src/operations/abstractoperation.cpp \
    src/operations/activityfetchoperation.cpp \
    src/operations/logfetchoperation.cpp \
    src/operations/sportssummaryoperation.cpp \
    src/operations/updatefirmwareoperation.cpp \
    src/devices/abstractdevice.cpp \
    src/devices/bipdevice.cpp \
    src/huamifirmwareinfo.cpp \
    src/dbushrm.cpp \
    src/activitysummary.cpp \
    src/deviceinterface.cpp \
    src/activitysample.cpp \
    src/activitykind.cpp \
    src/operations/sportsdetailoperation.cpp \
    src/operations/abstractfetchoperation.cpp \
    src/bipactivitydetailparser.cpp \
    src/activitycoordinate.cpp \
    src/sportsdatamodel.cpp \
    src/weather/city.cpp \
    src/weather/citymanager.cpp \
    src/weather/citysearchmodel.cpp \
    src/weather/currentweather.cpp \
    src/weather/huamiweathercondition.cpp

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
    qml/pages/SleepPage.qml

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
                translations/harbour-amazfish-sv.ts \
                translations/harbour-amazfish-es.ts
HEADERS += \
    src/qaesencryption.h \
    src/notificationslistener.h \
    src/voicecallhandler.h \
    src/voicecallmanager.h \
    src/typeconversion.h \
    src/settingsmanager.h \
    src/bipbatteryinfo.h \
    src/deviceinterface.h \
    src/datasource.h \
    src/activitykind.h \
    src/huamifirmwareinfo.h \
    src/dbushrm.h \
    src/activitysummary.h \ 
    src/activitysample.h \
    src/devicefactory.h \
    src/services/mibandservice.h \
    src/services/miband2service.h \
    src/services/alertnotificationservice.h \
    src/services/hrmservice.h \
    src/services/deviceinfoservice.h \
    src/services/bipfirmwareservice.h \
    src/operations/abstractoperation.h \
    src/operations/activityfetchoperation.h \
    src/operations/logfetchoperation.h \
    src/operations/sportssummaryoperation.h \
    src/operations/updatefirmwareoperation.h \   
    src/operations/sportsdetailoperation.h \
    src/operations/abstractfetchoperation.h \
    src/devices/abstractdevice.h \
    src/devices/bipdevice.h \
    src/bipactivitydetailparser.h \
    src/activitycoordinate.h \
    src/sportsdatamodel.h \
    src/weather/city.h \
    src/weather/citymanager.h \
    src/weather/citysearchmodel.h \
    src/weather/currentweather.h \
    src/weather/huamiweathercondition.h

