cmake_minimum_required(VERSION 3.10)
project(unikey-windows C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

set(UK_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/macrodialog.cpp
    src/trayicon.cpp
)

add_executable(UnikeyWayland WIN32 ${UK_SOURCES})
target_compile_definitions(UnikeyWayland PRIVATE -D_WIN32)

target_link_libraries(UnikeyWayland PRIVATE 
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network advapi32 user32
)

# Cài đặt file .exe vào thư mục bin
install(TARGETS UnikeyWayland DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
