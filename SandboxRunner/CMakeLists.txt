add_executable (SandboxRunner "SandboxRunner.cpp" "SandboxRunner.h" "../ThirdParty/cmdline.h")

target_include_directories(SandboxRunner PRIVATE ../ThirdParty)

find_package(stduuid CONFIG REQUIRED)
target_link_libraries(SandboxRunner PRIVATE sandbox stduuid)
sandboxrunner_configure_target(SandboxRunner)

add_dependencies(BUILD_ALL sandbox)
