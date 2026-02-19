#include "SandboxTest.h"

#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <type_traits>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace
{

struct SandboxConfigurationAbiBaseline
{
    const char *TaskName;
    const char *UserCommand;
    const char *WorkingDirectory;
    const char *const *EnvironmentVariables;
    uint16_t EnvironmentVariablesCount;
    const char *InputFile;
    const char *OutputFile;
    const char *ErrorFile;
    const char *LogFile;
    uint64_t MaxMemoryToCrash;
    uint64_t MaxMemory;
    uint64_t MaxStack;
    uint64_t MaxCpuTime;
    uint64_t MaxRealTime;
    uint64_t MaxOutputSize;
    int MaxProcessCount;
    int Policy;
};

struct SandboxResultAbiBaseline
{
    int Status;
    int ExitCode;
    int Signal;
    uint64_t CpuTimeUsage;
    uint64_t RealTimeUsage;
    uint64_t MemoryUsage;
};

using StartSandboxSignature           = int (*)(const SandboxConfiguration *, SandboxResult *);
using IsConfigurationValidSignature  = bool (*)(const SandboxConfiguration *);

static_assert(std::is_same_v<decltype(&StartSandbox), StartSandboxSignature>, "StartSandbox signature changed");
static_assert(std::is_same_v<decltype(&IsSandboxConfigurationVaild), IsConfigurationValidSignature>,
              "IsSandboxConfigurationVaild signature changed");

static_assert(DEFAULT == 0, "SandboxSecurePolicy::DEFAULT numeric value changed");
static_assert(CXX_PROGRAM == 1, "SandboxSecurePolicy::CXX_PROGRAM numeric value changed");

static_assert(SANDBOX_STATUS_SUCCESS == 0, "SANDBOX_STATUS_SUCCESS numeric value changed");
static_assert(SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED == 1, "SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED numeric value changed");
static_assert(SANDBOX_STATUS_RUNTIME_ERROR == 2, "SANDBOX_STATUS_RUNTIME_ERROR numeric value changed");
static_assert(SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED == 3,
              "SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED numeric value changed");
static_assert(SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED == 4,
              "SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED numeric value changed");
static_assert(SANDBOX_STATUS_PROCESS_LIMIT_EXCEEDED == 5,
              "SANDBOX_STATUS_PROCESS_LIMIT_EXCEEDED numeric value changed");
static_assert(SANDBOX_STATUS_OUTPUT_LIMIT_EXCEEDED == 6,
              "SANDBOX_STATUS_OUTPUT_LIMIT_EXCEEDED numeric value changed");
static_assert(SANDBOX_STATUS_ILLEGAL_OPERATION == 7, "SANDBOX_STATUS_ILLEGAL_OPERATION numeric value changed");
static_assert(SANDBOX_STATUS_INTERNAL_ERROR == 0xFFFF, "SANDBOX_STATUS_INTERNAL_ERROR numeric value changed");

static_assert(sizeof(SandboxConfiguration) == sizeof(SandboxConfigurationAbiBaseline),
              "SandboxConfiguration size changed");
static_assert(offsetof(SandboxConfiguration, TaskName) == offsetof(SandboxConfigurationAbiBaseline, TaskName),
              "SandboxConfiguration::TaskName offset changed");
static_assert(offsetof(SandboxConfiguration, UserCommand) == offsetof(SandboxConfigurationAbiBaseline, UserCommand),
              "SandboxConfiguration::UserCommand offset changed");
static_assert(offsetof(SandboxConfiguration, WorkingDirectory)
                  == offsetof(SandboxConfigurationAbiBaseline, WorkingDirectory),
              "SandboxConfiguration::WorkingDirectory offset changed");
static_assert(offsetof(SandboxConfiguration, EnvironmentVariables)
                  == offsetof(SandboxConfigurationAbiBaseline, EnvironmentVariables),
              "SandboxConfiguration::EnvironmentVariables offset changed");
static_assert(offsetof(SandboxConfiguration, EnvironmentVariablesCount)
                  == offsetof(SandboxConfigurationAbiBaseline, EnvironmentVariablesCount),
              "SandboxConfiguration::EnvironmentVariablesCount offset changed");
static_assert(offsetof(SandboxConfiguration, InputFile) == offsetof(SandboxConfigurationAbiBaseline, InputFile),
              "SandboxConfiguration::InputFile offset changed");
static_assert(offsetof(SandboxConfiguration, OutputFile) == offsetof(SandboxConfigurationAbiBaseline, OutputFile),
              "SandboxConfiguration::OutputFile offset changed");
static_assert(offsetof(SandboxConfiguration, ErrorFile) == offsetof(SandboxConfigurationAbiBaseline, ErrorFile),
              "SandboxConfiguration::ErrorFile offset changed");
static_assert(offsetof(SandboxConfiguration, LogFile) == offsetof(SandboxConfigurationAbiBaseline, LogFile),
              "SandboxConfiguration::LogFile offset changed");
static_assert(offsetof(SandboxConfiguration, MaxMemoryToCrash)
                  == offsetof(SandboxConfigurationAbiBaseline, MaxMemoryToCrash),
              "SandboxConfiguration::MaxMemoryToCrash offset changed");
static_assert(offsetof(SandboxConfiguration, MaxMemory) == offsetof(SandboxConfigurationAbiBaseline, MaxMemory),
              "SandboxConfiguration::MaxMemory offset changed");
static_assert(offsetof(SandboxConfiguration, MaxStack) == offsetof(SandboxConfigurationAbiBaseline, MaxStack),
              "SandboxConfiguration::MaxStack offset changed");
static_assert(offsetof(SandboxConfiguration, MaxCpuTime) == offsetof(SandboxConfigurationAbiBaseline, MaxCpuTime),
              "SandboxConfiguration::MaxCpuTime offset changed");
static_assert(offsetof(SandboxConfiguration, MaxRealTime) == offsetof(SandboxConfigurationAbiBaseline, MaxRealTime),
              "SandboxConfiguration::MaxRealTime offset changed");
static_assert(offsetof(SandboxConfiguration, MaxOutputSize)
                  == offsetof(SandboxConfigurationAbiBaseline, MaxOutputSize),
              "SandboxConfiguration::MaxOutputSize offset changed");
static_assert(offsetof(SandboxConfiguration, MaxProcessCount)
                  == offsetof(SandboxConfigurationAbiBaseline, MaxProcessCount),
              "SandboxConfiguration::MaxProcessCount offset changed");
static_assert(offsetof(SandboxConfiguration, Policy) == offsetof(SandboxConfigurationAbiBaseline, Policy),
              "SandboxConfiguration::Policy offset changed");

static_assert(sizeof(SandboxResult) == sizeof(SandboxResultAbiBaseline), "SandboxResult size changed");
static_assert(offsetof(SandboxResult, Status) == offsetof(SandboxResultAbiBaseline, Status),
              "SandboxResult::Status offset changed");
static_assert(offsetof(SandboxResult, ExitCode) == offsetof(SandboxResultAbiBaseline, ExitCode),
              "SandboxResult::ExitCode offset changed");
static_assert(offsetof(SandboxResult, Signal) == offsetof(SandboxResultAbiBaseline, Signal),
              "SandboxResult::Signal offset changed");
static_assert(offsetof(SandboxResult, CpuTimeUsage) == offsetof(SandboxResultAbiBaseline, CpuTimeUsage),
              "SandboxResult::CpuTimeUsage offset changed");
static_assert(offsetof(SandboxResult, RealTimeUsage) == offsetof(SandboxResultAbiBaseline, RealTimeUsage),
              "SandboxResult::RealTimeUsage offset changed");
static_assert(offsetof(SandboxResult, MemoryUsage) == offsetof(SandboxResultAbiBaseline, MemoryUsage),
              "SandboxResult::MemoryUsage offset changed");

std::filesystem::path FindSandboxLibraryPath()
{
    const std::filesystem::path cwd = std::filesystem::current_path();

#ifdef _WIN32
    const std::vector<std::filesystem::path> candidates = {
        cwd / "../SandboxRunnerCore/sandbox.dll",
        cwd / "SandboxRunnerCore/sandbox.dll",
        cwd / "sandbox.dll",
    };
#else
    const std::vector<std::filesystem::path> candidates = {
        cwd / "../SandboxRunnerCore/libsandbox.so",
        cwd / "SandboxRunnerCore/libsandbox.so",
        cwd / "libsandbox.so",
    };
#endif

    for (const auto &candidate : candidates)
    {
        if (std::filesystem::exists(candidate))
            return candidate;
    }

    return {};
}

} // namespace

TEST(AbiCompatibility, StructLayoutStable)
{
    EXPECT_EQ(sizeof(SandboxConfiguration), sizeof(SandboxConfigurationAbiBaseline));
    EXPECT_EQ(sizeof(SandboxResult), sizeof(SandboxResultAbiBaseline));
}

TEST(AbiCompatibility, ExportedSymbolsPresent)
{
    const std::filesystem::path libPath = FindSandboxLibraryPath();
    if (libPath.empty())
        GTEST_SKIP() << "sandbox shared library not found from test working directory";

#ifdef _WIN32
    HMODULE module = LoadLibraryA(libPath.string().c_str());
    ASSERT_NE(module, nullptr) << "LoadLibrary failed for " << libPath.string();

    const auto startSandboxFn = GetProcAddress(module, "StartSandbox");
    const auto validateFn     = GetProcAddress(module, "IsSandboxConfigurationVaild");

    EXPECT_NE(startSandboxFn, nullptr);
    EXPECT_NE(validateFn, nullptr);

    FreeLibrary(module);
#else
    void *handle = dlopen(libPath.string().c_str(), RTLD_NOW | RTLD_LOCAL);
    ASSERT_NE(handle, nullptr) << "dlopen failed for " << libPath.string() << ": " << dlerror();

    void *startSandboxFn = dlsym(handle, "StartSandbox");
    void *validateFn     = dlsym(handle, "IsSandboxConfigurationVaild");

    EXPECT_NE(startSandboxFn, nullptr);
    EXPECT_NE(validateFn, nullptr);

    dlclose(handle);
#endif
}
