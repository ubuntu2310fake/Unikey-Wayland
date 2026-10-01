cmake_minimum_required(VERSION 3.10)
project(unikey-wayland C CXX)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-DUKW_VERSION="3.0.2")

find_package(Qt6 COMPONENTS Core Gui Widgets DBus REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(UK_SOURCES
    src/mainwindow.cpp
    src/trayicon.cpp
    src/macrodialog.cpp
    src/windowtracker.cpp
    src/main.cpp
)

add_executable(unikey-wayland ${UK_SOURCES})
target_include_directories(unikey-wayland PRIVATE include)
target_compile_definitions(unikey-wayland PRIVATE -D_LINUX_)
target_compile_options(unikey-wayland PRIVATE -Wno-narrowing -include "${CMAKE_CURRENT_SOURCE_DIR}/include/windows_macros.h")
target_link_libraries(unikey-wayland PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus rt)

install(TARGETS unikey-wayland DESTINATION bin)
install(FILES ../io.github.ubuntu2310fake.UnikeyWayland.desktop DESTINATION share/applications)
install(FILES ../io.github.ubuntu2310fake.UnikeyWayland.svg DESTINATION share/icons/hicolor/scalable/apps)

# Ring 0 Daemon
add_executable(ukw_daemon ../ring0-engine/ukw_daemon.cpp)
target_link_libraries(ukw_daemon pthread)
install(TARGETS ukw_daemon DESTINATION bin)
install(FILES ../ring0-engine/dkms.conf DESTINATION src/ukw-driver-1.0)
install(FILES ../ring0-engine/Makefile DESTINATION src/ukw-driver-1.0)
install(FILES ../ring0-engine/ukw_driver.c DESTINATION src/ukw-driver-1.0)
install(FILES ../ring0-engine/ukw.service DESTINATION lib/systemd/system)
install(FILES ../ring0-engine/ukw.conf DESTINATION lib/modules-load.d)
install(FILES ../ring0-engine/xkb/ukw DESTINATION share/X11/xkb/symbols)
