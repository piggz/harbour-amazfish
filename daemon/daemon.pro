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
TARGET = harbour-amazfishd

CONFIG += sailfishapp
LIBS += -Lqble/qble -L$$OUT_PWD/../lib -lamazfish -lz

QT +=  contacts positioning KDb3

PKGCONFIG += mlite5
PKGCONFIG += libmkcal-qt5 libkcalcoren-qt5
INCLUDEPATH += /usr/include/mkcal-qt5 /usr/include/kcalcoren-qt5

INCLUDEPATH += $$PWD/src/services/ \
               $$PWD/src/operations/ \
               $$PWD/src/devices/ \
               $$PWD/src/ \
               $$PWD/../ \
               ../lib/src

target.path = /usr/bin/
systemd_services.path = /usr/lib/systemd/user/
systemd_services.files = harbour-amazfish.service

privilege.files = $${TARGET}.privileges
privilege.path = /usr/share/mapplauncherd/privileges.d/

INSTALLS += target \
            systemd_services \
            privilege


include(../qble/qble.pri)

SOURCES += \
    src/calendarreader.cpp \
    src/devices/abstractfirmwareinfo.cpp \
    src/devices/biplitedevice.cpp \
    src/devices/biplitefirmwareinfo.cpp \
    src/devices/gtsdevice.cpp \
    src/devices/gtsfirmwareinfo.cpp \
    src/devices/pinetimejfdevice.cpp \
    src/operations/updatefirmwareoperationnew.cpp \
    src/qaesencryption.cpp \
    src/services/currenttimeservice.cpp \
    src/voicecallhandler.cpp \
    src/voicecallmanager.cpp \
    src/typeconversion.cpp \
    src/bipbatteryinfo.cpp \
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
    src/devices/bipfirmwareinfo.cpp \
    src/dbushrm.cpp \
    src/activitysummary.cpp \
    src/deviceinterface.cpp \
    src/activitysample.cpp \
    src/activitykind.cpp \
    src/operations/sportsdetailoperation.cpp \
    src/operations/abstractfetchoperation.cpp \
    src/bipactivitydetailparser.cpp \
    src/activitycoordinate.cpp \
    src/harbour-amazfish-daemon.cpp \
    src/huamiweathercondition.cpp

DISTFILES += \
    harbour-amazfish.service \
    harbour-amazfishd.privileges

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

HEADERS += \
    src/calendarreader.h \
    src/devices/abstractfirmwareinfo.h \
    src/devices/biplitedevice.h \
    src/devices/biplitefirmwareinfo.h \
    src/devices/gtsdevice.h \
    src/devices/gtsfirmwareinfo.h \
    src/devices/pinetimejfdevice.h \
    src/operations/updatefirmwareoperationnew.h \
    src/qaesencryption.h \
    src/notificationslistener.h \
    src/services/currenttimeservice.h \
    src/voicecallhandler.h \
    src/voicecallmanager.h \
    src/typeconversion.h \
    src/bipbatteryinfo.h \
    src/deviceinterface.h \
    src/datasource.h \
    src/activitykind.h \
    src/devices/bipfirmwareinfo.h \
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

