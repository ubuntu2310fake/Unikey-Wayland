cmake_minimum_required(VERSION 3.10)
project(unikey-wayland C CXX)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DUKW_VERSION="2.0.4")

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_SCANNER REQUIRED wayland-scanner)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets DBus REQUIRED)

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
    src/mainwindow.cpp
    src/trayicon.cpp
    src/macrodialog.cpp
    src/windowtracker.cpp
    src/main.cpp
)

set(BAMBOO_LIB ${CMAKE_CURRENT_SOURCE_DIR}/src/libbamboo.a)
add_custom_command(
    OUTPUT ${BAMBOO_LIB} ${CMAKE_CURRENT_SOURCE_DIR}/src/libbamboo.h
    COMMAND go build -mod=vendor -buildmode=c-archive -o ${BAMBOO_LIB} bamboo_wrapper.go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/bamboo_wrapper.go
)
add_custom_target(bamboo_target DEPENDS ${BAMBOO_LIB})

add_executable(unikey-wayland ${PROTOCOL_C} ${UK_SOURCES})
target_include_directories(unikey-wayland PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR} 
    include
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
set_target_properties(unikey-wayland PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(unikey-wayland PRIVATE 
    ${WAYLAND_CLIENT_LIBRARIES}
    ${BAMBOO_LIB}
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus
    rt
)
add_dependencies(unikey-wayland bamboo_target)

# IBus Engine
pkg_check_modules(IBUS ibus-1.0)
if (IBUS_FOUND)
    set(IBUS_SOURCES
        ../ibus-engine/main.cpp
    )

    add_executable(ibus-engine-unikey-wayland ${IBUS_SOURCES})
    target_include_directories(ibus-engine-unikey-wayland PRIVATE 
        ${IBUS_INCLUDE_DIRS}
        include
        src
    )
    
    target_compile_definitions(ibus-engine-unikey-wayland PRIVATE 
        -D_LINUX_
    )
    target_compile_options(ibus-engine-unikey-wayland PRIVATE
        -Wno-narrowing
        -include "${CMAKE_CURRENT_SOURCE_DIR}/include/windows_macros.h"
        ${IBUS_CFLAGS_OTHER}
    )

    target_link_libraries(ibus-engine-unikey-wayland PRIVATE 
        ${IBUS_LIBRARIES}
        ${BAMBOO_LIB}
        rt
    )
    add_dependencies(ibus-engine-unikey-wayland bamboo_target)

    # Install rules
    install(TARGETS ibus-engine-unikey-wayland DESTINATION libexec)
    install(FILES ../ibus-engine/unikey-wayland.xml DESTINATION share/ibus/component)
    install(FILES ../ibus-engine/ibus-setup-unikey-wayland.desktop DESTINATION share/applications)
endif()

install(TARGETS unikey-wayland DESTINATION bin)
install(FILES ../unikey-wayland.desktop DESTINATION share/applications)
install(FILES ../unikey-wayland.metainfo.xml DESTINATION share/metainfo)



# X11 Native Client
set(X11_CLIENT_SOURCES
    ../x11-client/src/main.cpp
)

add_executable(unikey-x11 ${X11_CLIENT_SOURCES})
target_include_directories(unikey-x11 PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR} 
    include
    src
)

target_compile_definitions(unikey-x11 PRIVATE 
    -D_LINUX_
)
target_compile_options(unikey-x11 PRIVATE
    -Wno-narrowing
    -include "${CMAKE_CURRENT_SOURCE_DIR}/include/windows_macros.h"
)
set_target_properties(unikey-x11 PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(unikey-x11 PRIVATE 
    ${BAMBOO_LIB}
    X11 Xtst
)
add_dependencies(unikey-x11 bamboo_target)

install(TARGETS unikey-x11 DESTINATION bin)
