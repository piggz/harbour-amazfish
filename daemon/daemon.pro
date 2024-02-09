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

LIBS += -Lqble/qble -L$$OUT_PWD/../lib -lamazfish -lz
PKGCONFIG += dbus-1
PKGCONFIG += icu-uc icu-io icu-i18n # transliteration
QT +=  positioning KDb3 network dbus KArchive
CONFIG += c++17 c++1z

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
    message(SailfishOS daemon build)
    DEFINES += MER_EDITION_SAILFISH
    LIBS += -lkeepalive -lpulse-simple
    CONFIG += sailfishapp
    CONFIG += link_pkgconfig
    PKGCONFIG += mlite5 libpulse
    WATCHFISH_FEATURES += music \
                      voicecall \
                      notificationmonitor \
                      calendar
} else:flavor_uuitk {
    DEFINES += UUITK_EDITION
    WATCHFISH_FEATURES += music \
                      voicecall \
                      notificationmonitor \
                      calendar
} else {
WATCHFISH_FEATURES += music \
                  notificationmonitor \
                  calendar
}

INCLUDEPATH += /usr/include/mkcal-qt5 /usr/include/KF5/KCalendarCore /usr/include/KF5/KArchive

INCLUDEPATH += $$PWD/src/services/ \
               $$PWD/src/operations/ \
               $$PWD/src/devices/ \
               $$PWD/src/ \
               $$PWD/../ \
               ../lib/src

# PREFIX

isEmpty(PREFIX) {
    flavor_silica {
        PREFIX = /usr
    } else:flavor_uuitk {
        PREFIX = /
    } else {
        PREFIX = /usr/local
    }
}
message(The project will be installed in $$PREFIX)

target.path = $$PREFIX/bin
systemd_services.path = $$PREFIX/lib/systemd/user/
systemd_services.files = $$OUT_PWD/harbour-amazfish.service
systemd_services.CONFIG += no_check_exist

chirp.path = $$PREFIX/share/harbour-amazfish/
chirp.files = chirp.raw

#Install appropriate files for each system
flavor_silica {
    systemd_services.commands =  cp $$PWD/harbour-amazfish-sailfish.service.in $$OUT_PWD/harbour-amazfish.service

    privilege.files = $${TARGET}.privileges
    privilege.path = /usr/share/mapplauncherd/privileges.d/
    INSTALLS += privilege
} else {
    message(Configuring service file for regular linux))
    systemd_services.commands = sed \'s PREFIX $$PREFIX g\' $$PWD/harbour-amazfish.service.in > $$OUT_PWD/harbour-amazfish.service
    #systemd_services.commands =  cp $$PWD/harbour-amazfish.service.in $$OUT_PWD/harbour-amazfish.service; sed -i \'s PREFIX $$PREFIX g\' $$OUT_PWD/harbour-amazfish.service
}

INSTALLS += target \
            systemd_services \
            chirp

include(libwatchfish/libwatchfish.pri)
include(../qble/qble.pri)

SOURCES += \
    src/devices/asteroidosdevice.cpp \
    src/devices/banglejsdevice.cpp \
    src/devices/bipdevice.cpp \
    src/devices/bipsdevice.cpp \
    src/devices/gtr2device.cpp \
    src/devices/gtr2firmwareinfo.cpp \
    src/devices/gtrdevice.cpp \
    src/devices/gtrfirmwareinfo.cpp \
    src/devices/gts2device.cpp \
    src/devices/gts2firmwareinfo.cpp \
    src/devices/huamidevice.cpp \
    src/devices/huamifirmwareinfo.cpp \
    src/devices/infinitimefirmwareinfo.cpp \
    src/navigationinterface.cpp \
    src/devices/abstractfirmwareinfo.cpp \
    src/devices/biplitedevice.cpp \
    src/devices/biplitefirmwareinfo.cpp \
    src/devices/gtsdevice.cpp \
    src/devices/gtsfirmwareinfo.cpp \
    src/devices/pinetimejfdevice.cpp \
    src/operations/adafruitblefsoperation.cpp \
    src/operations/adafruitblefsworker.cpp \
    src/operations/dfuoperation.cpp \
    src/operations/dfuworker.cpp \
    src/operations/updatefirmwareoperationnew.cpp \
    src/operations/huamiupdatefirmwareoperation2020.cpp \
    src/qaesencryption.cpp \
    src/services/adafruitblefsservice.cpp \
    src/services/asteroidtimeservice.cpp \
    src/services/asteroidweatherservice.cpp \
    src/services/asteroidnotificationservice.cpp \
    src/services/asteroidmediaservice.cpp \
    src/services/asteroidscreenshotservice.cpp \
    src/services/currenttimeservice.cpp \
    src/services/dfuservice.cpp \
    src/services/infinitimemotionservice.cpp \
    src/services/infinitimenavservice.cpp \
    src/services/infinitimeweatherservice.cpp \
    src/services/pinetimesimpleweatherservice.cpp \
    src/services/pinetimemusicservice.cpp \
    src/services/uartservice.cpp \
    src/typeconversion.cpp \
    src/bipbatteryinfo.cpp \
    src/devicefactory.cpp \
    src/realtimeactivitysample.cpp \
    src/services/mibandservice.cpp \
    src/services/miband2service.cpp \
    src/services/alertnotificationservice.cpp \
    src/services/hrmservice.cpp \
    src/services/deviceinfoservice.cpp \
    src/services/bipfirmwareservice.cpp \
    src/services/batteryservice.cpp \
    src/services/immediatealertservice.cpp \
    src/operations/abstractoperation.cpp \
    src/operations/activityfetchoperation.cpp \
    src/operations/logfetchoperation.cpp \
    src/operations/sportssummaryoperation.cpp \
    src/operations/updatefirmwareoperation.cpp \
    src/devices/abstractdevice.cpp \
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
    src/transliterator.cpp \
    src/huamiweathercondition.cpp

DISTFILES += \
    chirp.raw \
    harbour-amazfish-sailfish.service.in \
    harbour-amazfish.service.in \
    harbour-amazfishd.privileges

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

HEADERS += \
    src/codec.h \
    src/devices/asteroidosdevice.h \
    src/devices/banglejsdevice.h \
    src/devices/bipdevice.h \
    src/devices/bipsdevice.h \
    src/devices/gtr2device.h \
    src/devices/gtr2firmwareinfo.h \
    src/devices/gtrdevice.h \
    src/devices/gtrfirmwareinfo.h \
    src/devices/gts2device.h \
    src/devices/gts2firmwareinfo.h \
    src/devices/huamidevice.h \
    src/devices/huamifirmwareinfo.h \
    src/devices/infinitimefirmwareinfo.h \
    src/navigationinterface.h \
    src/devices/abstractfirmwareinfo.h \
    src/devices/biplitedevice.h \
    src/devices/biplitefirmwareinfo.h \
    src/devices/gtsdevice.h \
    src/devices/gtsfirmwareinfo.h \
    src/devices/pinetimejfdevice.h \
    src/operations/adafruitblefsoperation.h \
    src/operations/adafruitblefsworker.h \
    src/operations/dfuoperation.h \
    src/operations/dfuworker.h \
    src/operations/updatefirmwareoperationnew.h \
    src/operations/huamiupdatefirmwareoperation2020.h \
    src/qaesencryption.h \
    src/services/adafruitblefsservice.h \
    src/services/asteroidtimeservice.h \
    src/services/asteroidweatherservice.h \
    src/services/asteroidnotificationservice.h \
    src/services/asteroidmediaservice.h \
    src/services/asteroidscreenshotservice.h \
    src/services/currenttimeservice.h \
    src/services/dfuservice.h \
    src/services/infinitimemotionservice.h \
    src/services/infinitimenavservice.h \
    src/services/infinitimeweatherservice.h \
    src/services/pinetimesimpleweatherservice.h \
    src/services/pinetimemusicservice.h \
    src/services/uartservice.h \
    src/services/immediatealertservice.h \
    src/typeconversion.h \
    src/bipbatteryinfo.h \
    src/deviceinterface.h \
    src/activitykind.h \
    src/devices/bipfirmwareinfo.h \
    src/dbushrm.h \
    src/activitysummary.h \ 
    src/activitysample.h \
    src/devicefactory.h \
    src/realtimeactivitysample.h \
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
    src/services/batteryservice.h \
    src/devices/abstractdevice.h \
    src/bipactivitydetailparser.h \
    src/transliterator.h \
    src/activitycoordinate.h
