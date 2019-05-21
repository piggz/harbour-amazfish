#
# tohkbd2 top level project file
#
TEMPLATE = subdirs
CONFIG = ordered
SUBDIRS = daemon ui
OTHER_FILES =

DISTFILES += \
    rpm/harbour-amazfish.changes.in \
    rpm/harbour-amazfish.changes.run.in \
    rpm/harbour-amazfish.spec \
<<<<<<< HEAD
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
				translations/harbour-amazfish-nl.ts \
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

=======
    rpm/harbour-amazfish.yaml \
>>>>>>> daemon
