add_subdirectory("Samples")
add_subdirectory("SanitizerSamples")

add_executable(SandboxTest
        SandboxTest.cpp
        SandboxTest.h
        AbiCompatibilityTest.cpp
        PolicyRegistryTest.cpp
        ResourceConfigTest.cpp
        SandboxRunnerCliTest.cpp
        SanitizerSandboxTest.cpp)

enable_testing()

target_link_libraries(SandboxTest PRIVATE sandbox)
sandboxrunner_configure_target(SandboxTest)
add_dependencies(SandboxTest SAMPLES_TESTS)
add_dependencies(SandboxTest SANITIZER_SAMPLES_TESTS)

find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(SandboxTest PRIVATE GTest::gtest GTest::gmock GTest::gmock_main nlohmann_json::nlohmann_json)
add_dependencies(SandboxTest SandboxRunner)

include(GoogleTest)
gtest_discover_tests(SandboxTest
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests
        DISCOVERY_TIMEOUT 30)
file(COPY ${CMAKE_SOURCE_DIR}/policies/CXX_PROGRAM.json DESTINATION ${CMAKE_BINARY_DIR}/Tests)

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
