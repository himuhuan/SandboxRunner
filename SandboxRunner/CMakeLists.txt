add_executable (SandboxRunner "SandboxRunner.cpp" "SandboxRunner.h" "../ThirdParty/cmdline.h")

target_include_directories(SandboxRunner PRIVATE ../ThirdParty)

find_package(stduuid CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(SandboxRunner PRIVATE sandbox stduuid nlohmann_json::nlohmann_json)
sandboxrunner_configure_target(SandboxRunner)

add_dependencies(BUILD_ALL sandbox)
