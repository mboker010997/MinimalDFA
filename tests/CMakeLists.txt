cmake_minimum_required(VERSION 3.5)

project(AutTests)

find_package(GTest REQUIRED CONFIG)

add_executable(Tests main_test.cpp tests_imp.cpp)
add_dependencies(Tests AutLib)
add_dependencies(Tests copy_test_files)

target_include_directories(
    Tests
    PUBLIC
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(
    Tests
    PUBLIC
    AutLib
    GTest::gtest GTest::gtest_main
)




# target_include_directories(
#     Tests
#     PUBLIC
#     ../lib
# )

include(GoogleTest)
gtest_discover_tests(Tests)