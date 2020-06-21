cmake_minimum_required(VERSION 3.16)
project(SPLP)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_EXE src/main.cpp)

set(SOURCE_LIB src/splp.cpp src/splp.h src/splp_solver.cpp src/splp_solver.h)
add_library(lib STATIC ${SOURCE_LIB})

add_executable(SPLP ${SOURCE_EXE})

target_link_libraries(SPLP lib)