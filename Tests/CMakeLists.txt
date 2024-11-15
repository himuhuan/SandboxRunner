cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory("Samples")

add_executable(SandboxTest SandboxTest.cpp SandboxTest.h)

enable_testing()

target_link_libraries(SandboxTest PRIVATE sandbox)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(SandboxTest PRIVATE GTest::gtest GTest::gmock GTest::gmock_main)

add_test(AllTestsInMain SandboxTest)