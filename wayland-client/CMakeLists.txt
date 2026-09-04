cmake_minimum_required(VERSION 3.16)
project(unikey-wayland LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
find_package(Qt6 COMPONENTS Core Gui Widgets DBus REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_program(WAYLAND_SCANNER_BIN wayland-scanner REQUIRED)
set(PROTOCOL_XML ${CMAKE_CURRENT_SOURCE_DIR}/protocols/input-method-unstable-v1.xml)
set(PROTOCOL_C ${CMAKE_CURRENT_BINARY_DIR}/input-method-unstable-v1-protocol.c)
set(PROTOCOL_H ${CMAKE_CURRENT_BINARY_DIR}/input-method-unstable-v1-client-protocol.h)

add_custom_command(
    OUTPUT ${PROTOCOL_C}
    COMMAND ${WAYLAND_SCANNER_BIN} private-code ${PROTOCOL_XML} ${PROTOCOL_C}
    DEPENDS ${PROTOCOL_XML}
)
add_custom_command(
    OUTPUT ${PROTOCOL_H}
    COMMAND ${WAYLAND_SCANNER_BIN} client-header ${PROTOCOL_XML} ${PROTOCOL_H}
    DEPENDS ${PROTOCOL_XML}
)

find_program(GO_BIN go REQUIRED)
set(BAMBOO_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/libbamboo.a)
file(GLOB_RECURSE BAMBOO_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.go
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/*.go
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/modules.txt
)
add_custom_command(
    OUTPUT ${BAMBOO_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/src/libbamboo.h
    COMMAND ${GO_BIN} build -mod=vendor -buildmode=c-archive -o ${BAMBOO_LIB} bamboo_wrapper.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    DEPENDS ${BAMBOO_SOURCES}
)
add_custom_target(bamboo_target DEPENDS ${BAMBOO_LIB})

set(UK_SOURCES
    src/trayicon.cpp
    src/mainwindow.cpp
    src/macrodialog.cpp
    src/windowtracker.cpp
    src/main.cpp
    src/text_transaction.h
)
add_executable(unikey-wayland
    ${UK_SOURCES}
    ${PROTOCOL_C}
    ${PROTOCOL_H}
)
add_dependencies(unikey-wayland bamboo_target)
target_include_directories(unikey-wayland PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)
target_compile_definitions(unikey-wayland PRIVATE _LINUX_)
target_link_libraries(unikey-wayland PRIVATE
    PkgConfig::WAYLAND_CLIENT
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    ${BAMBOO_LIB}
)

# IBus backend for GNOME Wayland and X11 sessions.
pkg_check_modules(IBUS IMPORTED_TARGET ibus-1.0)
if (IBUS_FOUND)
    add_executable(ibus-engine-unikey-wayland ../ibus-engine/main.cpp)
    add_dependencies(ibus-engine-unikey-wayland bamboo_target)
    target_include_directories(ibus-engine-unikey-wayland PRIVATE src)
    target_compile_definitions(ibus-engine-unikey-wayland PRIVATE _LINUX_)
    target_link_libraries(ibus-engine-unikey-wayland PRIVATE
        PkgConfig::IBUS
        ${BAMBOO_LIB}
    )

    install(TARGETS ibus-engine-unikey-wayland DESTINATION libexec)
    install(FILES ../ibus-engine/unikey-wayland.xml DESTINATION share/ibus/component)
    install(FILES ../ibus-engine/ibus-setup-unikey-wayland.desktop DESTINATION share/applications)
endif()

install(TARGETS unikey-wayland DESTINATION bin)
install(FILES ../io.github.ubuntu2310fake.UnikeyWayland.desktop DESTINATION share/applications)
install(FILES ../io.github.ubuntu2310fake.UnikeyWayland.metainfo.xml DESTINATION share/metainfo)
install(FILES ../io.github.ubuntu2310fake.UnikeyWayland.svg DESTINATION share/icons/hicolor/scalable/apps)

include(CTest)
if (BUILD_TESTING)
    add_executable(text-transaction-test tests/text_transaction_test.cpp)
    target_include_directories(text-transaction-test PRIVATE src)
    add_test(NAME text-transaction-test COMMAND text-transaction-test)
    add_test(
        NAME bamboo-wrapper-test
        COMMAND ${GO_BIN} test -mod=vendor bamboo_wrapper.go bamboo_wrapper_test.go
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endif()
