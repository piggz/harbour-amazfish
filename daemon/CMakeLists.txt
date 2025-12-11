set(SOURCES
./src/achievements.cpp
./src/activitycoordinate.cpp
./src/activitysample.cpp
./src/activitysummary.cpp
./src/devices/bipactivitydetailparser.cpp
./src/bipbatteryinfo.cpp
./src/dbushrm.cpp
./src/devicefactory.cpp
./src/deviceinterface.cpp
./src/devices/abstractdevice.cpp
./src/devices/abstractfirmwareinfo.cpp
./src/devices/asteroidosdevice.cpp
./src/devices/banglejsdevice.cpp
./src/devices/bipdevice.cpp
./src/devices/bipfirmwareinfo.cpp
./src/devices/biplitedevice.cpp
./src/devices/biplitefirmwareinfo.cpp
./src/devices/bipsdevice.cpp
./src/devices/dk08device.cpp
./src/devices/gtr2device.cpp
./src/devices/gtr2firmwareinfo.cpp
./src/devices/gtrdevice.cpp
./src/devices/gtrfirmwareinfo.cpp
./src/devices/gts2device.cpp
./src/devices/gts2firmwareinfo.cpp
./src/devices/gtsdevice.cpp
./src/devices/gtsfirmwareinfo.cpp
./src/devices/huamidevice.cpp
./src/devices/huamifirmwareinfo.cpp
./src/devices/infinitimefirmwareinfo.cpp
./src/devices/neodevice.cpp
./src/devices/neofirmwareinfo.cpp
./src/devices/pebbledevice.cpp
./src/devices/pinetimejfdevice.cpp
./src/devices/zepposdevice.cpp
./src/devices/abstractactivitydetailparser.cpp
./src/devices/huamiactivitysummaryparser.cpp
./src/devices/zeppos/abstractzepposservice.cpp
./src/devices/zeppos/huami2021chunkedencoder.cpp
./src/devices/zeppos/huami2021chunkeddecoder.cpp
./src/devices/zeppos/zepposservicesservice.cpp
./src/devices/zeppos/zepposnotificationservice.cpp
./src/devices/zeppos/zepposagpsservice.cpp
./src/devices/zeppos/zepposauthservice.cpp
./src/devices/zeppos/zepposbatteryservice.cpp
./src/devices/zeppos/zepposstepsservice.cpp
./src/devices/zeppos/huami2021handler.cpp
./src/devices/zeppos/zepposheartrateservice.cpp
./src/devices/zeppos/zepposactivitysummaryparser.cpp
./src/devices/zeppos/zepposactivitydetailparser.cpp
./src/devices/zeppos/zeppostimeservice.cpp
./src/devices/zeppos/zepposuserinfoservice.cpp
./src/devices/zeppos/picoproto.cpp
./src/devices/zeppos/zepposfiletransferservice.cpp
./src/devices/zeppos/filetransfer/zepposfiletransferimpl.cpp
./src/devices/zeppos/filetransfer/zepposfiletransferv3.cpp
./src/harbour-amazfish-daemon.cpp
./src/huamiweathercondition.cpp
./src/navigationinterface.cpp
./src/operations/abstractfetchoperation.cpp
./src/operations/abstractoperation.cpp
./src/operations/activityfetchoperation.cpp
./src/operations/adafruitblefsoperation.cpp
./src/operations/adafruitblefsworker.cpp
./src/operations/dfuoperation.cpp
./src/operations/dfuworker.cpp
./src/operations/huamiupdatefirmwareoperation2020.cpp
./src/operations/logfetchoperation.cpp
./src/operations/sportsdetailoperation.cpp
./src/operations/sportssummaryoperation.cpp
./src/operations/updatefirmwareoperation.cpp
./src/operations/updatefirmwareoperationnew.cpp
./src/operations/zeppos/abstractzepposoperation.cpp
./src/operations/zeppos/zepposagpsupdateoperation.cpp
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
./src/devices/asteroidosdevice.h
./src/devices/banglejsdevice.h
./src/devices/bipdevice.h
./src/devices/bipfirmwareinfo.h
./src/devices/biplitedevice.h
./src/devices/biplitefirmwareinfo.h
./src/devices/bipsdevice.h
./src/devices/dk08device.h
./src/devices/gtr2device.h
./src/devices/gtr2firmwareinfo.h
./src/devices/gtrdevice.h
./src/devices/gtrfirmwareinfo.h
./src/devices/gts2device.h
./src/devices/gts2firmwareinfo.h
./src/devices/gtsdevice.h
./src/devices/gtsfirmwareinfo.h
./src/devices/huamidevice.h
./src/devices/huamifirmwareinfo.h
./src/devices/infinitimefirmwareinfo.h
./src/devices/neodevice.h
./src/devices/neofirmwareinfo.h
./src/devices/pebbledevice.h
./src/devices/pinetimejfdevice.h
./src/devices/zepposdevice.h
./src/devices/bipactivitydetailparser.h
./src/devices/huamiactivitysummaryparser.h
./src/devices/zeppos/abstractzepposservice.h
./src/devices/zeppos/huami2021chunkedencoder.h
./src/devices/zeppos/huami2021chunkeddecoder.h
./src/devices/zeppos/zepposservicesservice.h
./src/devices/zeppos/zepposnotificationservice.h
./src/devices/zeppos/zepposagpsservice.h
./src/devices/zeppos/zepposauthservice.h
./src/devices/zeppos/zepposbatteryservice.h
./src/devices/zeppos/zepposstepsservice.h
./src/devices/zeppos/zepposheartrateservice.h
./src/devices/zeppos/huami2021handler.h
./src/devices/zeppos/picoproto.h
./src/devices/zeppos/zepposactivitysummaryparser.h
./src/devices/zeppos/zepposactivitydetailparser.h
./src/devices/zeppos/zeppostimeservice.h
./src/devices/zeppos/zepposuserinfoservice.h
./src/devices/zeppos/zepposfiletransferservice.h
./src/devices/zeppos/filetransfer/zepposfiletransferimpl.h
./src/devices/zeppos/filetransfer/zepposfiletransferv3.h
./src/huamiweathercondition.h
./src/navigationinterface.h
./src/operations/abstractfetchoperation.h
./src/operations/abstractoperation.h
./src/operations/activityfetchoperation.h
./src/operations/adafruitblefsoperation.h
./src/operations/adafruitblefsworker.h
./src/operations/dfuoperation.h
./src/operations/dfuworker.h
./src/operations/huamiupdatefirmwareoperation2020.h
./src/operations/logfetchoperation.h
./src/operations/sportsdetailoperation.h
./src/operations/sportssummaryoperation.h
./src/operations/updatefirmwareoperation.h
./src/operations/updatefirmwareoperationnew.h
./src/operations/zeppos/abstractzepposoperation.h
./src/operations/zeppos/zepposagpsupdateoperation.h
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
    src/devices/zepposfirmwareinfo.h src/devices/zepposfirmwareinfo.cpp
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
