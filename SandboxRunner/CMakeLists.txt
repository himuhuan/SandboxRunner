add_executable (SandboxRunner "SandboxRunner.cpp" "SandboxRunner.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SandboxRunner PROPERTY CXX_STANDARD 20)
endif()

include_directories(../ThirdParty)

find_package(stduuid CONFIG REQUIRED)
target_link_libraries(SandboxRunner PRIVATE sandbox stduuid)

add_dependencies(BUILD_ALL sandbox)