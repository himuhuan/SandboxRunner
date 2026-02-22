add_library(sandbox SHARED
        Sandbox.h Sandbox.cpp "Linux/LinuxSandboxImpl.cpp" "Logger.h" "Logger.cpp"
        Linux/SandboxChildProcess.cpp Linux/SandboxChildProcess.h
        SandboxUtils.cpp
        SandboxUtils.h
        Linux/SecurePolicy.cpp
        Linux/SecurePolicy.h
        Linux/SandboxMonitor.cpp
        Linux/SandboxMonitor.h
        Linux/ErrorHandler.h
        Linux/ErrorHandler.cpp
        InternalHelpers.h
        InternalHelpers.cpp)

find_package(fmt REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(Threads REQUIRED)

target_link_libraries(sandbox PRIVATE fmt::fmt spdlog::spdlog seccomp Threads::Threads)
sandboxrunner_configure_target(sandbox)

add_dependencies(BUILD_ALL sandbox)
