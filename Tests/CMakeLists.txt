add_subdirectory("Samples")

add_executable(SandboxTest
        SandboxTest.cpp
        SandboxTest.h
        AbiCompatibilityTest.cpp
        PolicyRegistryTest.cpp
        ResourceConfigTest.cpp)

enable_testing()

target_link_libraries(SandboxTest PRIVATE sandbox)
sandboxrunner_configure_target(SandboxTest)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(SandboxTest PRIVATE GTest::gtest GTest::gmock GTest::gmock_main)

add_test(AllTestsInMain SandboxTest)
set_tests_properties(AllTestsInMain PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests)

find_program(DOTNET_EXECUTABLE NAMES dotnet)
if (DOTNET_EXECUTABLE)
    add_test(NAME PInvokeSmoke
             COMMAND ${DOTNET_EXECUTABLE} run --project ${CMAKE_CURRENT_SOURCE_DIR}/PInvokeSmoke/PInvokeSmoke.csproj --configuration Release -- ${CMAKE_BINARY_DIR}/Tests)

    if (WIN32)
        set_tests_properties(PInvokeSmoke PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests
                             ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/SandboxRunnerCore;$ENV{PATH}")
    else ()
        set_tests_properties(PInvokeSmoke PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests
                             ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/SandboxRunnerCore")
    endif ()
endif ()
