cmake_minimum_required(VERSION 3.10)
project(unikey-wayland C CXX)

set(CMAKE_CXX_STANDARD 11)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_SCANNER REQUIRED wayland-scanner)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Wayland protocol generation
pkg_get_variable(WAYLAND_SCANNER_BIN wayland-scanner wayland_scanner)
set(PROTOCOL_XML ${CMAKE_CURRENT_SOURCE_DIR}/protocols/input-method-unstable-v1.xml)
set(PROTOCOL_C ${CMAKE_CURRENT_BINARY_DIR}/input-method-unstable-v1-protocol.c)
set(PROTOCOL_H ${CMAKE_CURRENT_BINARY_DIR}/input-method-unstable-v1-client-protocol.h)

add_custom_command(
    OUTPUT ${PROTOCOL_C} ${PROTOCOL_H}
    COMMAND ${WAYLAND_SCANNER_BIN} private-code ${PROTOCOL_XML} ${PROTOCOL_C}
    COMMAND ${WAYLAND_SCANNER_BIN} client-header ${PROTOCOL_XML} ${PROTOCOL_H}
    DEPENDS ${PROTOCOL_XML}
)

# UniKey sources
set(UK_SOURCES
    ../keyhook/vietkey.cpp
    ../newkey/encode.cpp
    ../vnconv/convert.cpp
    ../vnconv/charset.cpp
    ../vnconv/data.cpp
    ../vnconv/pattern.cpp
    ../byteio/byteio.cpp
    src/ukengine_wrapper.cpp
    src/mainwindow.cpp
    src/trayicon.cpp
    src/macrodialog.cpp
    src/main.cpp
)

add_executable(unikey-wayland ${PROTOCOL_C} ${UK_SOURCES})
target_include_directories(unikey-wayland PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR} 
    include
    ../vnconv
    ../keyhook
    ../newkey
    ../byteio
)

# Add preprocessor definition so it avoids MFC/Windows headers if possible, 
# though we supply our own windows.h
target_compile_definitions(unikey-wayland PRIVATE 
    -D_LINUX_
)
target_compile_options(unikey-wayland PRIVATE
    -Wno-narrowing
    -include "${CMAKE_CURRENT_SOURCE_DIR}/include/windows_macros.h"
)

target_link_libraries(unikey-wayland PRIVATE 
    ${WAYLAND_CLIENT_LIBRARIES}
    Qt6::Core Qt6::Gui Qt6::Widgets
)
