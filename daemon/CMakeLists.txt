set(SOURCES
./src/activitycoordinate.cpp
./src/activitykind.cpp
./src/activitysample.cpp
./src/activitysummary.cpp
./src/bipactivitydetailparser.cpp
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
./src/devices/pinetimejfdevice.cpp
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
./src/qaesencryption.cpp
./src/realtimeactivitysample.cpp
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
./src/services/hrmservice.cpp
./src/services/immediatealertservice.cpp
./src/services/infinitimemotionservice.cpp
./src/services/infinitimenavservice.cpp
#./src/services/infinitimeresourceservice.cpp
./src/services/infinitimeweatherservice.cpp
./src/services/miband2service.cpp
./src/services/mibandservice.cpp
./src/services/pinetimemusicservice.cpp
./src/services/pinetimesimpleweatherservice.cpp
./src/services/uartservice.cpp
./src/transliterator.cpp
./src/typeconversion.cpp
)

set(HEADERS
./src/activitycoordinate.h
./src/activitykind.h
./src/activitysample.h
./src/activitysummary.h
./src/bipactivitydetailparser.h
./src/bipbatteryinfo.h
./src/codec.h
./src/dbushrm.h
./src/devicefactory.h
./src/deviceinterface.h
./src/devices/abstractdevice.h
./src/devices/abstractfirmwareinfo.h
./src/devices/asteroidosdevice.h
./src/devices/banglejsdevice.h
./src/devices/bipdevice.h
./src/devices/bipfirmwareinfo.h
./src/devices/biplitedevice.h
./src/devices/biplitefirmwareinfo.h
./src/devices/bipsdevice.h
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
./src/devices/pinetimejfdevice.h
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
./src/qaesencryption.h
./src/realtimeactivitysample.h
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
./src/services/hrmservice.h
./src/services/immediatealertservice.h
./src/services/infinitimemotionservice.h
./src/services/infinitimenavservice.h
#./src/services/infinitimeresourceservice.h
./src/services/infinitimeweatherservice.h
./src/services/miband2service.h
./src/services/mibandservice.h
./src/services/pinetimemusicservice.h
./src/services/pinetimesimpleweatherservice.h
./src/services/uartservice.h
./src/transliterator.h
./src/typeconversion.h
)

add_executable(harbour-amazfishd ${SOURCES} ${HEADERS})

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Gui Widgets Positioning Network REQUIRED)
find_package(KF5Archive REQUIRED)
find_package(ZLIB REQUIRED)
find_package(ICU REQUIRED COMPONENTS uc i18n data)
find_package(KDb REQUIRED)

if(FLAVOR STREQUAL "silica")
    set(WATCHFISH_FEATURES "notificationmonitor;music;calendar;voicecall")

    pkg_check_modules(PULSE REQUIRED libpulse)
    target_include_directories(harbour-amazfishd PUBLIC ${PULSE_INCLUDE_DIRS})
    target_link_libraries(harbour-amazfishd PUBLIC ${PULSE_LIBRARIES} pulse-simple)

    pkg_check_modules(MLITE5 REQUIRED mlite5)
    target_include_directories(harbour-amazfishd PUBLIC ${MLITE5_INCLUDE_DIRS})
    target_link_libraries(harbour-amazfishd PUBLIC ${MLITE5_LIBRARIES})

    target_link_libraries(harbour-amazfishd PUBLIC keepalive)

elseif(FLAVOR STREQUAL "uuitk")
    set(WATCHFISH_FEATURES "notificationmonitor;music;calendar;voicecall")

    pkg_check_modules(PULSE REQUIRED libpulse)
    target_include_directories(harbour-amazfishd PUBLIC ${PULSE_INCLUDE_DIRS})
    target_link_libraries(harbour-amazfishd PUBLIC ${PULSE_LIBRARIES} pulse-simple)


else()
    set(WATCHFISH_FEATURES "notificationmonitor;music;calendar")
endif()

add_subdirectory(libwatchfish)



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
elseif(FLAVOR STREQUAL "silica")
    add_definitions(-DTRANSLATION_FOLDER="/usr/share/harbour-amazfishd/translations")
else() 
    add_definitions(-DTRANSLATION_FOLDER=".")
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
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/harbour-amazfish.service
    DESTINATION  ${CMAKE_INSTALL_PREFIX}/lib/systemd/user)
