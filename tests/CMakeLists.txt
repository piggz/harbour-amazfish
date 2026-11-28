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

# UI screenshots: the daemon and the app start without a screen, the app opens every page and saves it to
# screenshots/NAME.png. A QML error on any page fails the test. Build with -DDEMO_DEVICE=ON for a connected watch.
# With xvfb-run the app draws with OpenGL (Mesa llvmpipe); the offscreen platform skips shader effects.
find_program(DBUS_RUN_SESSION dbus-run-session)
find_program(XVFB_RUN xvfb-run)
if(XVFB_RUN)
    set(SCREENSHOT_DISPLAY ${XVFB_RUN} -a -s "-screen 0 1440x900x24")
    set(SCREENSHOT_PLATFORM xcb)
else()
    set(SCREENSHOT_DISPLAY "")
    set(SCREENSHOT_PLATFORM offscreen)
endif()
if(DBUS_RUN_SESSION)
    # With the KDE desktop style and the Breeze color schemes, the pages look as on Plasma, in light and dark.
    set(SCREENSHOT_THEMES light)
    set(SCREENSHOT_STYLE "")
    if(EXISTS "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_QML}/org/kde/desktop/qmldir"
        AND EXISTS "/usr/share/color-schemes/BreezeDark.colors")
        set(SCREENSHOT_THEMES light dark)
        set(SCREENSHOT_STYLE "QT_QUICK_CONTROLS_STYLE=org.kde.desktop;")
    endif()
    foreach(THEME ${SCREENSHOT_THEMES})
        if(THEME STREQUAL "dark")
            set(SCREENSHOT_TEST ui_screenshots_dark)
            set(SCREENSHOT_DIR screenshots-dark)
            set(SCREENSHOT_SCHEME BreezeDark)
        else()
            set(SCREENSHOT_TEST ui_screenshots)
            set(SCREENSHOT_DIR screenshots)
            set(SCREENSHOT_SCHEME BreezeLight)
        endif()
        add_test(NAME ${SCREENSHOT_TEST} COMMAND ${SCREENSHOT_DISPLAY} ${DBUS_RUN_SESSION} --
            ${CMAKE_CURRENT_SOURCE_DIR}/run-with-daemon.sh $<TARGET_FILE:harbour-amazfishd> $<TARGET_FILE:harbour-amazfish-ui>)
        set_tests_properties(${SCREENSHOT_TEST} PROPERTIES
            TIMEOUT 180
            ENVIRONMENT "QT_QPA_PLATFORM=${SCREENSHOT_PLATFORM};QSG_RENDER_LOOP=basic;LC_ALL=en_US.UTF-8;${SCREENSHOT_STYLE}AMAZFISH_COLOR_SCHEME=${SCREENSHOT_SCHEME};HOME=${CMAKE_CURRENT_BINARY_DIR}/home;AMAZFISH_SCREENSHOT_STEPS=${CMAKE_CURRENT_SOURCE_DIR}/screenshots.steps;AMAZFISH_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}/${SCREENSHOT_DIR}"
            FAIL_REGULAR_EXPRESSION "is not installed;is not a type;unavailable;ReferenceError;TypeError;Screenshot steps:")
    endforeach()
endif()
