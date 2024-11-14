add_library(sandbox SHARED
        Sandbox.h Sandbox.cpp "Linux/LinuxSandboxImpl.cpp" "Logger.h" "Logger.cpp"
        Linux/SandboxChildProcess.cpp Linux/SandboxChildProcess.h
        SandboxUtils.cpp
        SandboxUtils.h
        Linux/SecurePolicy.cpp
        Linux/SecurePolicy.h
        Linux/SandboxMonitor.cpp
        Linux/SandboxMonitor.h)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET sandbox PROPERTY CXX_STANDARD 20)
endif ()

find_package(fmt REQUIRED CONFIG)
target_link_libraries(sandbox PRIVATE fmt::fmt seccomp pthread)

add_dependencies(BUILD_ALL sandbox)