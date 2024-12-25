set(o2_WITH_KEYCHAIN OFF CACHE BOOL "keychain store" FORCE)
add_subdirectory(o2)


file(GLOB TS_FILES translations/*.ts)

if(FLAVOR STREQUAL "uuitk")
    add_definitions(-DTRANSLATION_FOLDER="./share/harbour-amazfish/translations")
else()
    add_definitions(-DTRANSLATION_FOLDER=".")
endif()

list(APPEND RESOURCES amazfish.qrc)
list(APPEND RESOURCES icons.qrc)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/platform-${FLAVOR}.qrc")
    list(APPEND RESOURCES "platform-${FLAVOR}.qrc")
else()
    message(WARNING "Resource file 'platform-${FLAVOR}.qrc' not found. Proceeding without it.")
endif()

set(SOURCES
    ./src/datasource.cpp
    ./src/sportsdatamodel.cpp
    ./src/sportsmetamodel.cpp
    ./src/timeformatter.cpp
    ./src/harbour-amazfish-ui.cpp
    ./src/trackloader.cpp
    ./src/daemoninterface.cpp
)

set(HEADERS
    ./src/datasource.h
    ./src/sportsdatamodel.h
    ./src/sportsmetamodel.h
    ./src/timeformatter.h
    ./src/trackloader.h
    ./src/daemoninterface.h
)

add_executable(harbour-amazfish-ui
  ${SOURCES}
  ${HEADERS}
  ${RESOURCES}
  ${TS_FILES}
)

find_package(Qt5 COMPONENTS Gui Widgets Quick Positioning REQUIRED)
find_package(KDb REQUIRED)

QT5_ADD_TRANSLATION(QM_FILES ${TS_FILES})
add_custom_target(ui_translations DEPENDS ${QM_FILES})
add_dependencies(harbour-amazfish-ui ui_translations)

QT5_CREATE_TRANSLATION(QT_FILES "${CMAKE_SOURCE_DIR}"
    OPTIONS "-no-obsolete")

install(FILES ${QM_FILES}
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/harbour-amazfish/translations")

install(TARGETS harbour-amazfish-ui RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR})

target_include_directories(harbour-amazfish-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(harbour-amazfish-ui
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(harbour-amazfish-ui
  PRIVATE 
  libamazfish libwatchfish qble 
  Qt5::Core Qt5::Widgets Qt5::Quick Qt5::Positioning
  KDb
  o2
)

#install(FILES "harbour-amazfish.desktop"
#    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")

#install(FILES "icons/256x256/harbour-amazfish.png"
#    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps")

#install(FILES "icons/512x512/harbour-amazfish.png"
#    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps")

# install(FILES "harbour-amazfish.appdata.xml"
#    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo")
