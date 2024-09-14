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

CONFIG-=qtquickcompiler

QT +=  positioning KDb3 quick
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
include (o2/src/src.pri)

SOURCES += src/harbour-amazfish-ui.cpp \
    src/datasource.cpp \
    src/sportsdatamodel.cpp \
    src/daemoninterface.cpp \
    src/sportsmetamodel.cpp \
    src/timeformatter.cpp \
    src/trackloader.cpp

DISTFILES += qml/harbour-amazfish.qml \
    qml/components/DateNavigation.qml \
    qml/components/DeviceButton.qml \
    qml/components/PercentCircle.qml \
    qml/components/TruncationModes.qml \
    qml/components/platform.silica/ApplicationWindowPL.qml \
    qml/components/platform.silica/BusyIndicatorPL.qml \
    qml/components/platform.silica/BusyIndicatorSmallPL.qml \
    qml/components/platform.silica/ButtonPL.qml \
    qml/components/platform.silica/ClipboardPL.qml \
    qml/components/platform.silica/ComboBoxPL.qml \
    qml/components/platform.silica/ContextMenuItemPL.qml \
    qml/components/platform.silica/ContextMenuPL.qml \
    qml/components/platform.silica/Cover.qml \
    qml/components/platform.silica/DatePickerDialogPL.qml \
    qml/components/platform.silica/DialogAutoPL.qml \
    qml/components/platform.silica/DialogListPL.qml \
    qml/components/platform.silica/DialogPL.qml \
    qml/components/platform.silica/ExpandingSectionGroupPL.qml \
    qml/components/platform.silica/ExpandingSectionPL.qml \
    qml/components/platform.silica/FileSelectorPL.qml \
    qml/components/platform.silica/FormLayoutPL.qml \
    qml/components/platform.silica/IconButtonPL.qml \
    qml/components/platform.silica/IconPL.qml \
    qml/components/platform.silica/LabelPL.qml \
    qml/components/platform.silica/ListItemPL.qml \
    qml/components/platform.silica/MenuDrawerItemPL.qml \
    qml/components/platform.silica/MenuDrawerPL.qml \
    qml/components/platform.silica/MenuDrawerSubmenuItemPL.qml \
    qml/components/platform.silica/MenuDrawerSubmenuPL.qml \
    qml/components/platform.silica/PageEmptyPL.qml \
    qml/components/platform.silica/PageListPL.qml \
    qml/components/platform.silica/PageMenuItemPL.qml \
    qml/components/platform.silica/PageMenuPL.qml \
    qml/components/platform.silica/PagePL.qml \
    qml/components/platform.silica/RemorsePopupPL.qml \
    qml/components/platform.silica/SearchFieldPL.qml \
    qml/components/platform.silica/SectionHeaderPL.qml \
    qml/components/platform.silica/SliderPL.qml \
    qml/components/platform.silica/StackPL.qml \
    qml/components/platform.silica/StylerPL.qml \
    qml/components/platform.silica/TextAreaPL.qml \
    qml/components/platform.silica/TextFieldPL.qml \
    qml/components/platform.silica/TextSwitchPL.qml \
    qml/components/platform.silica/TimePickerDialogPL.qml \
    qml/components/platform.silica/ToolItemPL.qml \
    qml/components/platform.silica/ValueButtonPL.qml \
    qml/cover/CoverPage.qml \
    qml/pages/AddCityPage.qml \
    qml/pages/FirstPage.qml \
    qml/pages/HeartratePage.qml \
    qml/pages/PairSelectDeviceType.qml \
    qml/pages/Settings-button-action.qml \
    qml/pages/Settings-huami-shortcuts.qml \
    qml/pages/SportPage.qml \
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
    qml/components/Alarm.qml \
    qml/pages/Settings-alarms.qml \
    qml/pages/BipFirmwarePage.qml \
    qml/pages/SportsSummaryPage.qml \
    qml/pages/AnalysisPage.qml \
    qml/pages/StepsPage.qml \
    qml/pages/SleepPage.qml \
    qml/components/DownloadDataMenuItem.qml \
    qml/pages/AuthKeyDialog.qml \
    qml/pages/UnpairDeviceDialog.qml \
    qml/pages/StravaSettingsPage.qml \
    qml/pages/StravaUploadPage.qml \
    harbour-amazfish-ui.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += \
    translations/harbour-amazfish-ui-cs.ts \
    translations/harbour-amazfish-ui-de.ts \
    translations/harbour-amazfish-ui-es.ts \
    translations/harbour-amazfish-ui-fr.ts \
    translations/harbour-amazfish-ui-nl.ts \
    translations/harbour-amazfish-ui-pl.ts \
    translations/harbour-amazfish-ui-ru.ts \
    translations/harbour-amazfish-ui-sv.ts \
    translations/harbour-amazfish-ui-zh_CN.ts

HEADERS += \
    src/datasource.h \
    src/daemoninterface.h \ \
    src/sportsdatamodel.h \
    src/sportsmetamodel.h \
    src/timeformatter.h \
    src/trackloader.h

RESOURCES += \
    amazfish.qrc \
    icons.qrc

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

equals(DISABLE_SYSTEMD, "yes") {
    DEFINES += DISABLE_SYSTEMD
}

flavor_uuitk {
    DEFINES += TRANSLATION_FOLDER=\\\"./translations\\\"
} else {
    DEFINES += TRANSLATION_FOLDER=\\\"$${PREFIX}/share/$${TARGET}/translations\\\"
}

flavor_silica {
    message(SailfishOS build)
    CONFIG += sailfishapp sailfishapp_no_deploy_qml sailfishapp_i18n
    DEFINES += MER_EDITION_SAILFISH

    qtPrepareTool(LRELEASE, lrelease)
    for(tsfile, TRANSLATIONS) {
        qmfile = $$shadowed($$tsfile)
        qmfile ~= s,.ts$,.qm,
        qmdir = $$dirname(qmfile)
        !exists($$qmdir) {
            mkpath($$qmdir)|error("Aborting.")
        }
        command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
        system($$command)|error("Failed to run: $$command")
        TRANSLATIONS_FILES += $$qmfile
    }

    translations_files.files = $${TRANSLATIONS_FILES}
    translations_files.path = $${PREFIX}/share/$${TARGET}/translations
    INSTALLS += translations_files
}

flavor_uuitk {
    message(UUITK build)
    DEFINES += UUITK_EDITION

    qtPrepareTool(LRELEASE, lrelease)
    for(tsfile, TRANSLATIONS) {
        qmfile = $$shadowed($$tsfile)
        qmfile ~= s,.ts$,.qm,
        qmdir = $$dirname(qmfile)
        !exists($$qmdir) {
            mkpath($$qmdir)|error("Aborting.")
        }
        command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
        system($$command)|error("Failed to run: $$command")
        TRANSLATIONS_FILES += $$qmfile
    }
    translations_files.files = $${TRANSLATIONS_FILES}
    translations_files.path = /translations
    INSTALLS += translations_files
}

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

# PREFIX_RUNNING
isEmpty(PREFIX_RUNNING) {
    flavor_uuitk {
        PREFIX_RUNNING = .
    } else {
        PREFIX_RUNNING = $$PREFIX
    }
}

DATADIR = $$PREFIX/share/$${TARGET}

target.path = $$PREFIX/bin

qml.files = qml/*.qml \
            qml/pages \
            qml/cover \
            qml/components
qml.path = $$DATADIR/qml

js.files = qml/tools/*.js
js.path = $$DATADIR/qml/tools

icons.files = qml/pics/*.png qml/pics/devices  qml/custom-icons
icons.path = $$DATADIR/qml/pics

qmlplatform.extra = mkdir -p ${INSTALL_ROOT}$$DATADIR/qml/components/platform && cp -L -v $$PWD/qml/components/platform.$$FLAVOR/*.qml ${INSTALL_ROOT}$$DATADIR/qml/components/platform
qmlplatform.path = $$DATADIR/qml/platform

desktopfile.files = harbour-amazfish-ui.desktop
desktopfile.path = $$PREFIX/share/applications

appicon.files = ../harbour-amazfish-ui.svg
appicon.path = $$PREFIX/share/icons/hicolor/scalable/apps

INSTALLS += qmlplatform qml js icons target desktopfile appicon

#End install config
