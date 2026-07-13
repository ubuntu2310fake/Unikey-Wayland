cmake_minimum_required(VERSION 3.10)
project(UnikeyBamboo)

set(CMAKE_CXX_STANDARD 17)

# Tạo file EXE chạy ngầm không có cửa sổ console (WIN32)
add_executable(UnikeyBamboo WIN32 main.cpp)

# Không cần link User32 rõ ràng trong CMake Windows, nhưng cứ khai báo cho chắc
target_link_libraries(UnikeyBamboo User32)

# Create install step
install(TARGETS UnikeyBamboo DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
