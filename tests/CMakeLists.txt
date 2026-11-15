find_package(Qt${QT_MAJOR} REQUIRED COMPONENTS Test QuickTest Qml)

# C++ unit tests
add_executable(tst_typeconversion cpp/tst_typeconversion.cpp ${CMAKE_SOURCE_DIR}/daemon/src/typeconversion.cpp)
target_include_directories(tst_typeconversion PRIVATE ${CMAKE_SOURCE_DIR}/daemon/src)
target_link_libraries(tst_typeconversion PRIVATE Qt${QT_MAJOR}::Core Qt${QT_MAJOR}::Test)
add_test(NAME tst_typeconversion COMMAND tst_typeconversion)

# QML tests: the components are loaded from the same resources as in the app.
add_executable(tst_qml qml/tst_qml.cpp ${CMAKE_SOURCE_DIR}/ui/platform-${FLAVOR}.qrc ${CMAKE_SOURCE_DIR}/ui/icons.qrc)
target_compile_definitions(tst_qml PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml")
target_link_libraries(tst_qml PRIVATE Qt${QT_MAJOR}::Qml Qt${QT_MAJOR}::QuickTest)
add_test(NAME tst_qml COMMAND tst_qml)
set_tests_properties(tst_qml PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# UI screenshots: the app starts offscreen, opens every page and saves it to screenshots/NAME.png.
# A QML error on any page fails the test.
find_program(DBUS_RUN_SESSION dbus-run-session)
if(DBUS_RUN_SESSION)
    add_test(NAME ui_screenshots COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:harbour-amazfish-ui>)
    set_tests_properties(ui_screenshots PROPERTIES
        TIMEOUT 180
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;LC_ALL=en_US.UTF-8;HOME=${CMAKE_CURRENT_BINARY_DIR}/home;AMAZFISH_SCREENSHOT_STEPS=${CMAKE_CURRENT_SOURCE_DIR}/screenshots.steps;AMAZFISH_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}/screenshots"
        FAIL_REGULAR_EXPRESSION "is not installed;is not a type;unavailable;ReferenceError;TypeError;Screenshot steps:")
endif()
