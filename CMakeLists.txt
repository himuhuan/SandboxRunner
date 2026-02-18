cmake_minimum_required(VERSION 3.12)

if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()

project("SandboxRunner")

add_library(sandboxrunner_project_options INTERFACE)
target_compile_features(sandboxrunner_project_options INTERFACE cxx_std_20)

add_library(sandboxrunner_warnings INTERFACE)
if (MSVC)
    target_compile_options(sandboxrunner_warnings INTERFACE /W4 /permissive-)
else ()
    target_compile_options(sandboxrunner_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif ()

function(sandboxrunner_configure_target target_name)
    target_link_libraries(${target_name} PRIVATE sandboxrunner_project_options sandboxrunner_warnings)
    target_compile_definitions(${target_name} PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)
endfunction()

enable_testing()

add_custom_target(BUILD_ALL)

add_subdirectory("SandboxRunner")
add_subdirectory("SandboxRunnerCore")
add_subdirectory("Tests")
