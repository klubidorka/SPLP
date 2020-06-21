cmake_minimum_required(VERSION 3.16)
project(SPLP)

set(CMAKE_CXX_STANDARD 14)

set(BOOST_ROOT "/Users/klubidorka/Desktop/MIPT-6sem/ACO/boost_1_57_0/")

find_package(Boost 1.57.0)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

add_executable(SPLP main.cpp splp.cpp splp.h splp_solver.cpp splp_solver.h)