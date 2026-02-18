set(SOURCES
./src/achievements.cpp
./src/activitycoordinate.cpp
./src/activitysample.cpp
./src/activitysummary.cpp
./src/bipbatteryinfo.cpp
./src/dbushrm.cpp
./src/devicefactory.cpp
./src/deviceinterface.cpp
./src/devices/abstractdevice.cpp
./src/devices/abstractfirmwareinfo.cpp
./src/devices/huami/bipdevice.cpp
./src/devices/huami/bipfirmwareinfo.cpp
./src/devices/huami/biplitedevice.cpp
./src/devices/huami/biplitefirmwareinfo.cpp
./src/devices/huami/bipsdevice.cpp
./src/devices/huami/bipactivitydetailparser.cpp
./src/devices/huami/gtr2device.cpp
./src/devices/huami/gtr2firmwareinfo.cpp
./src/devices/huami/gtrdevice.cpp
./src/devices/huami/gtrfirmwareinfo.cpp
./src/devices/huami/gts2device.cpp
./src/devices/huami/gts2firmwareinfo.cpp
./src/devices/huami/gtsdevice.cpp
./src/devices/huami/gtsfirmwareinfo.cpp
./src/devices/huami/huamidevice.cpp
./src/devices/huami/huamifirmwareinfo.cpp
./src/devices/huami/neodevice.cpp
./src/devices/huami/neofirmwareinfo.cpp
./src/devices/huami/zepposdevice.cpp
./src/devices/huami/zepposfirmwareinfo.cpp
./src/devices/abstractactivitydetailparser.cpp
./src/devices/huami/huamiactivitysummaryparser.cpp
./src/devices/huami/huamifetcher.cpp
./src/devices/huami/zeppos/abstractzepposservice.cpp
./src/devices/huami/zeppos/huami2021chunkedencoder.cpp
./src/devices/huami/zeppos/huami2021chunkeddecoder.cpp
./src/devices/huami/zeppos/zepposservicesservice.cpp
./src/devices/huami/zeppos/zepposnotificationservice.cpp
./src/devices/huami/zeppos/zepposagpsservice.cpp
./src/devices/huami/zeppos/zepposauthservice.cpp
./src/devices/huami/zeppos/zepposbatteryservice.cpp
./src/devices/huami/zeppos/zepposstepsservice.cpp
./src/devices/huami/zeppos/huami2021handler.cpp
./src/devices/huami/zeppos/zepposheartrateservice.cpp
./src/devices/huami/zeppos/zepposactivitysummaryparser.cpp
./src/devices/huami/zeppos/zepposactivitydetailparser.cpp
./src/devices/huami/zeppos/zeppostimeservice.cpp
./src/devices/huami/zeppos/zepposuserinfoservice.cpp
./src/devices/huami/zeppos/picoproto.cpp
./src/devices/huami/zeppos/zepposfiletransferservice.cpp
./src/devices/huami/zeppos/filetransfer/zepposfiletransferimpl.cpp
./src/devices/huami/zeppos/filetransfer/zepposfiletransferv3.cpp
./src/devices/asteroidosdevice.cpp
./src/devices/banglejsdevice.cpp
./src/devices/pebbledevice.cpp
./src/devices/pinetimejfdevice.cpp
./src/devices/infinitimefirmwareinfo.cpp
./src/devices/dk08device.cpp
./src/devices/huami/uihhcontainer.cpp
./src/harbour-amazfish-daemon.cpp
./src/huamiweathercondition.cpp
./src/navigationinterface.cpp
./src/operations/abstractfetchoperation.cpp
./src/operations/abstractoperation.cpp
./src/operations/huami/activityfetchoperation.cpp
./src/operations/huami/huamiupdatefirmwareoperation2020.cpp
./src/operations/huami/logfetchoperation.cpp
./src/operations/huami/sportsdetailoperation.cpp
./src/operations/huami/sportssummaryoperation.cpp
./src/operations/huami/updatefirmwareoperation.cpp
./src/operations/huami/updatefirmwareoperationnew.cpp
./src/operations/huami/zeppos/abstractzepposoperation.cpp
./src/operations/huami/zeppos/zepposagpsupdateoperation.cpp
./src/operations/huami/fetchhrvoperation.cpp
./src/operations/huami/fetchpaioperation.cpp
./src/operations/huami/fetchspo2normaloperation.cpp
./src/operations/huami/fetchspo2sleepoperation.cpp
./src/operations/adafruitblefsoperation.cpp
./src/operations/adafruitblefsworker.cpp
./src/operations/dfuoperation.cpp
./src/operations/dfuworker.cpp
./src/qaesencryption.cpp
./src/realtimeactivitysample.cpp
./src/services/abstractoperationservice.cpp
./src/services/adafruitblefsservice.cpp
./src/services/alertnotificationservice.cpp
./src/services/asteroidmediaservice.cpp
./src/services/asteroidnotificationservice.cpp
./src/services/asteroidscreenshotservice.cpp
./src/services/asteroidtimeservice.cpp
./src/services/asteroidweatherservice.cpp
./src/services/batteryservice.cpp
./src/services/bipfirmwareservice.cpp
./src/services/currenttimeservice.cpp
./src/services/deviceinfoservice.cpp
./src/services/dfuservice.cpp
./src/services/dk08nusservice.cpp
./src/services/dk08wechatservice.cpp
./src/services/hrmservice.cpp
./src/services/immediatealertservice.cpp
./src/services/infinitimemotionservice.cpp
./src/services/infinitimenavservice.cpp
#./src/services/infinitimeresourceservice.cpp
./src/services/infinitimeweatherservice.cpp
./src/services/miband2service.cpp
./src/services/mibandservice.cpp
./src/services/pebbleservice.cpp
./src/services/pinetimemusicservice.cpp
./src/services/pinetimesimpleweatherservice.cpp
./src/services/uartservice.cpp
./src/transliterator.cpp
./src/typeconversion.cpp
./src/ecdh/ecdh.c
)

set(HEADERS
./src/achievements.h
./src/activitycoordinate.h
./src/activitysample.h
./src/activitysummary.h
./src/bipbatteryinfo.h
./src/codec.h
./src/dbushrm.h
./src/devicefactory.h
./src/deviceinterface.h
./src/devices/abstractdevice.h
./src/devices/abstractfirmwareinfo.h
./src/devices/abstractactivitysummaryparser.h
./src/devices/abstractactivitydetailparser.h
./src/devices/huami/bipdevice.h
./src/devices/huami/bipfirmwareinfo.h
./src/devices/huami/biplitedevice.h
./src/devices/huami/biplitefirmwareinfo.h
./src/devices/huami/bipsdevice.h
./src/devices/huami/gtr2device.h
./src/devices/huami/gtr2firmwareinfo.h
./src/devices/huami/gtrdevice.h
./src/devices/huami/gtrfirmwareinfo.h
./src/devices/huami/gts2device.h
./src/devices/huami/gts2firmwareinfo.h
./src/devices/huami/gtsdevice.h
./src/devices/huami/gtsfirmwareinfo.h
./src/devices/huami/neodevice.h
./src/devices/huami/neofirmwareinfo.h
./src/devices/huami/huamidevice.h
./src/devices/huami/huamifirmwareinfo.h
./src/devices/huami/zepposdevice.h
./src/devices/huami/zepposfirmwareinfo.h
./src/devices/huami/uihhcontainer.h
./src/devices/huami/bipactivitydetailparser.h
./src/devices/huami/huamiactivitysummaryparser.h
./src/devices/huami/huamifetcher.h
./src/devices/huami/zeppos/abstractzepposservice.h
./src/devices/huami/zeppos/huami2021chunkedencoder.h
./src/devices/huami/zeppos/huami2021chunkeddecoder.h
./src/devices/huami/zeppos/zepposservicesservice.h
./src/devices/huami/zeppos/zepposnotificationservice.h
./src/devices/huami/zeppos/zepposagpsservice.h
./src/devices/huami/zeppos/zepposauthservice.h
./src/devices/huami/zeppos/zepposbatteryservice.h
./src/devices/huami/zeppos/zepposstepsservice.h
./src/devices/huami/zeppos/zepposheartrateservice.h
./src/devices/huami/zeppos/huami2021handler.h
./src/devices/huami/zeppos/picoproto.h
./src/devices/huami/zeppos/zepposactivitysummaryparser.h
./src/devices/huami/zeppos/zepposactivitydetailparser.h
./src/devices/huami/zeppos/zeppostimeservice.h
./src/devices/huami/zeppos/zepposuserinfoservice.h
./src/devices/huami/zeppos/zepposfiletransferservice.h
./src/devices/huami/zeppos/filetransfer/zepposfiletransferimpl.h
./src/devices/huami/zeppos/filetransfer/zepposfiletransferv3.h
./src/devices/asteroidosdevice.h
./src/devices/banglejsdevice.h
./src/devices/pebbledevice.h
./src/devices/pinetimejfdevice.h
./src/devices/infinitimefirmwareinfo.h
./src/devices/dk08device.h
./src/huamiweathercondition.h
./src/navigationinterface.h
./src/operations/abstractfetchoperation.h
./src/operations/abstractoperation.h
./src/operations/huami/activityfetchoperation.h
./src/operations/huami/huamiupdatefirmwareoperation2020.h
./src/operations/huami/logfetchoperation.h
./src/operations/huami/sportsdetailoperation.h
./src/operations/huami/sportssummaryoperation.h
./src/operations/huami/updatefirmwareoperation.h
./src/operations/huami/updatefirmwareoperationnew.h
./src/operations/huami/zeppos/abstractzepposoperation.h
./src/operations/huami/zeppos/zepposagpsupdateoperation.h
./src/operations/huami/fetchhrvoperation.h
./src/operations/huami/fetchpaioperation.h
./src/operations/huami/fetchspo2normaloperation.h
./src/operations/huami/fetchspo2sleepoperation.h
./src/operations/adafruitblefsoperation.h
./src/operations/adafruitblefsworker.h
./src/operations/dfuoperation.h
./src/operations/dfuworker.h
./src/qaesencryption.h
./src/realtimeactivitysample.h
./src/services/abstractoperationservice.h
./src/services/adafruitblefsservice.h
./src/services/alertnotificationservice.h
./src/services/asteroidmediaservice.h
./src/services/asteroidnotificationservice.h
./src/services/asteroidscreenshotservice.h
./src/services/asteroidtimeservice.h
./src/services/asteroidweatherservice.h
./src/services/batteryservice.h
./src/services/bipfirmwareservice.h
./src/services/currenttimeservice.h
./src/services/deviceinfoservice.h
./src/services/dfuservice.h
./src/services/dk08nusservice.h
./src/services/dk08wechatservice.h
./src/services/hrmservice.h
./src/services/immediatealertservice.h
./src/services/infinitimemotionservice.h
./src/services/infinitimenavservice.h
#./src/services/infinitimeresourceservice.h
./src/services/infinitimeweatherservice.h
./src/services/miband2service.h
./src/services/mibandservice.h
./src/services/pebbleservice.h
./src/services/pinetimemusicservice.h
./src/services/pinetimesimpleweatherservice.h
./src/services/uartservice.h
./src/transliterator.h
./src/typeconversion.h
./src/ecdh/ecdh.h
)

add_executable(harbour-amazfishd ${SOURCES} ${HEADERS}
)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Gui Widgets Positioning Network REQUIRED)
find_package(KF5Archive REQUIRED)
find_package(ZLIB REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n data)
find_package(KDb REQUIRED)

if(FLAVOR STREQUAL "silica")
    set(WATCHFISH_FEATURES "notificationmonitor;music;calendar;voicecall;soundprofile")

    pkg_check_modules(PULSE REQUIRED libpulse)
    target_include_directories(harbour-amazfishd PUBLIC ${PULSE_INCLUDE_DIRS})
    target_link_libraries(harbour-amazfishd PUBLIC ${PULSE_LIBRARIES} pulse-simple)

    pkg_check_modules(MLITE5 REQUIRED mlite5)
    target_include_directories(harbour-amazfishd PUBLIC ${MLITE5_INCLUDE_DIRS})
    target_link_libraries(harbour-amazfishd PUBLIC ${MLITE5_LIBRARIES})

    target_link_libraries(harbour-amazfishd PUBLIC keepalive)

elseif(FLAVOR STREQUAL "uuitk")
    set(WATCHFISH_FEATURES "notificationmonitor;music;calendar;voicecall;soundprofile")

    pkg_check_modules(PULSE REQUIRED libpulse)
    target_include_directories(harbour-amazfishd PUBLIC ${PULSE_INCLUDE_DIRS})
    target_link_libraries(harbour-amazfishd PUBLIC ${PULSE_LIBRARIES} pulse-simple)

    pkg_check_modules(LIBUSERMETRICSINPUT REQUIRED libusermetricsinput-1)

    target_include_directories(harbour-amazfishd PUBLIC ${LIBUSERMETRICSINPUT_INCLUDE_DIRS})
    target_link_libraries(harbour-amazfishd PUBLIC ${LIBUSERMETRICSINPUT_LIBRARIES})
else()
    set(WATCHFISH_FEATURES "notificationmonitor;music;calendar;soundprofile")
endif()

add_subdirectory(libwatchfish)
add_subdirectory(Qt-AES)

install(TARGETS harbour-amazfishd RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR})

target_include_directories(harbour-amazfishd PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/devices
    ${CMAKE_CURRENT_SOURCE_DIR}/src/operations
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(harbour-amazfishd
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(harbour-amazfishd
  PRIVATE
  libwatchfish libamazfish qble
  Qt5::Gui Qt5::Widgets Qt5::Core Qt5::Positioning Qt5::Network
  KF5::Archive
  ZLIB::ZLIB
  ICU::uc ICU::i18n ICU::data
  KDb
)


## translations
file(GLOB TS_FILES translations/*.ts)

# TODO check if translation path is correct on all platforms
if(FLAVOR STREQUAL "uuitk")
    add_definitions(-DTRANSLATION_FOLDER="../share/harbour-amazfishd/translations")
else()
    add_definitions(-DTRANSLATION_FOLDER="${CMAKE_INSTALL_FULL_DATAROOTDIR}/harbour-amazfishd/translations")
endif()

QT5_ADD_TRANSLATION(QM_FILES ${TS_FILES})
add_custom_target(daemon_translations DEPENDS ${QM_FILES})
add_dependencies(harbour-amazfishd daemon_translations)

QT5_CREATE_TRANSLATION(QT_FILES "${CMAKE_SOURCE_DIR}"
    OPTIONS "-no-obsolete")

install(FILES ${QM_FILES}
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/harbour-amazfishd/translations")

install(FILES "chirp.raw"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/harbour-amazfishd/")

install(FILES "harbour-amazfishd.privileges"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/mapplauncherd/privileges.d/")

if(FLAVOR STREQUAL "silica")
    configure_file(harbour-amazfish-sailfish.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/harbour-amazfish.service)
else()
    configure_file(harbour-amazfish.service.in
        ${CMAKE_CURRENT_BINARY_DIR}/harbour-amazfish.service)
endif()

set(SYSTEMD_USER_UNIT_DIR "lib/systemd/user" CACHE PATH "Directory for systemd user units")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/harbour-amazfish.service
    DESTINATION ${SYSTEMD_USER_UNIT_DIR})
