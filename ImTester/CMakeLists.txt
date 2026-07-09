cmake_minimum_required(VERSION 3.16)
project(ImTester)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# GUI Target
add_executable(ImTester main.cpp)
target_link_libraries(ImTester Qt6::Core Qt6::Gui Qt6::Widgets)

# CLI Helper Target
add_executable(im-helper im-helper.cpp)
target_link_libraries(im-helper)
