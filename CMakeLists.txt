cmake_minimum_required(VERSION 3.5)

project(Automaton)

add_subdirectory(lib)
add_subdirectory(tests)

add_executable(main main.cpp)
add_dependencies(main AutLib)

target_include_directories(
    main
    PUBLIC
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(
    main
    PUBLIC
    AutLib
)
