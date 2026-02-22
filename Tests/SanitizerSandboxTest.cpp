#include "SandboxTest.h"

#include <filesystem>
#include <limits>
#include <string>

namespace
{

SandboxConfiguration BuildSanitizerConfig(const char *taskName,
                                          const std::string &binaryPath,
                                          const std::string &inputPath,
                                          const std::string &outputPath,
                                          uint64_t maxMemoryBytes,
                                          uint64_t maxMemoryToCrashBytes)
{
    SandboxConfiguration configuration{};
    configuration.TaskName        = taskName;
    configuration.UserCommand     = binaryPath.c_str();
    configuration.InputFile       = inputPath.c_str();
    configuration.OutputFile      = outputPath.c_str();
    configuration.MaxRealTime     = 3000;
    configuration.MaxCpuTime      = 1000;
    configuration.MaxMemory       = maxMemoryBytes;
    configuration.MaxMemoryToCrash = maxMemoryToCrashBytes;
    configuration.MaxOutputSize   = 10 * 1024;
    configuration.MaxProcessCount = 0;
    configuration.Policy          = CXX_PROGRAM;
    return configuration;
}

} // namespace

TEST(SanitizerSandboxTest, UbsanInstrumentedBinaryAccepted)
{
    const auto currentDirectory = std::filesystem::current_path();
    const std::string executable = (currentDirectory / "SanitizerSamples" / "ExpectedAcceptedUbsan").string();
    const std::string inputFile  = (currentDirectory / "TestData" / "test_data.in").string();
    const std::string outputFile = (currentDirectory / "TestData" / "ExpectedAcceptedUbsan.out").string();

    SandboxResult result{};
    auto configuration = BuildSanitizerConfig("ExpectedAcceptedUbsan",
                                              executable,
                                              inputFile,
                                              outputFile,
                                              128ULL * 1024 * 1024,
                                              0);

    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    ASSERT_EQ(result.ExitCode, 0);
    ASSERT_EQ(result.Signal, 0);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_SUCCESS);
}

TEST(SanitizerSandboxTest, AsanAndUbsanInstrumentedBinaryAcceptedWithHigherMemoryBudget)
{
    const auto currentDirectory = std::filesystem::current_path();
    const std::string executable = (currentDirectory / "SanitizerSamples" / "ExpectedAcceptedAsanUbsan").string();
    const std::string inputFile  = (currentDirectory / "TestData" / "test_data.in").string();
    const std::string outputFile = (currentDirectory / "TestData" / "ExpectedAcceptedAsanUbsan.out").string();

    SandboxResult result{};
    auto configuration = BuildSanitizerConfig("ExpectedAcceptedAsanUbsan",
                                              executable,
                                              inputFile,
                                              outputFile,
                                              512ULL * 1024 * 1024,
                                              std::numeric_limits<uint64_t>::max());
    const char *const environmentVariables[] = {
        "ASAN_OPTIONS=detect_leaks=0",
        "UBSAN_OPTIONS=print_stacktrace=0",
        nullptr,
    };
    configuration.EnvironmentVariables = environmentVariables;
    configuration.EnvironmentVariablesCount = 2;

    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    ASSERT_EQ(result.ExitCode, 0);
    ASSERT_EQ(result.Signal, 0);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_SUCCESS);
}
