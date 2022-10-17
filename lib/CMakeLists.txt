cmake_minimum_required(VERSION 3.5)

project(AutLibBuild)

add_library(AutLib SHARED automaton.cpp)

target_include_directories(
    AutLib
    PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_include_directories(
    AutLib
    PUBLIC
    ${CMAKE_SOURCE_DIR}/tests
)