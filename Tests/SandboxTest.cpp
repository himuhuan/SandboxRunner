/**
 * SandboxTest.cpp -- Test cases for the sandbox library
 *
 * @file SandboxTest.cpp
 * This file is part of the SandboxRunner project.
 */

#include "SandboxTest.h"
#include <filesystem>
#include <string_view>

#define INIT_SANDBOX_TESTCASE(TestName)                                                                                \
    SandboxResult result{};                                                                                            \
    SandboxConfiguration configuration{};                                                                              \
    configuration.TaskName        = #TestName;                                                                         \
    auto currentDirectory         = std::filesystem::current_path();                                                   \
    std::string executable        = (currentDirectory / "Samples" / #TestName).string();                               \
    std::string inputFile         = (currentDirectory / "TestData" / "test_data.in").string();                         \
    std::string outputFile        = (currentDirectory / "TestData" / #TestName ".out").string();                       \
    configuration.UserCommand     = executable.c_str();                                                                \
    configuration.MaxRealTime     = 3000;                                                                              \
    configuration.InputFile       = inputFile.c_str();                                                                 \
    configuration.OutputFile      = outputFile.c_str();                                                                \
    configuration.MaxMemory       = 128 * 1024 * 1024;                                                                 \
    configuration.MaxCpuTime      = 1000;                                                                              \
    configuration.MaxOutputSize   = 10 * 1024;                                                                         \
    configuration.MaxProcessCount = 0;                                                                                 \
    configuration.Policy          = CXX_PROGRAM

std::string_view GetStatusName(SandboxStatus status)
{
    switch (status)
    {
    case SANDBOX_STATUS_SUCCESS:
        return "SANDBOX_STATUS_SUCCESS";
    case SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED:
        return "SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_RUNTIME_ERROR:
        return "SANDBOX_STATUS_RUNTIME_ERROR";
    case SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED:
        return "SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED:
        return "SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_PROCESS_LIMIT_EXCEEDED:
        return "SANDBOX_STATUS_PROCESS_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_OUTPUT_LIMIT_EXCEEDED:
        return "SANDBOX_STATUS_OUTPUT_LIMIT_EXCEEDED";
    default:
        return "SANDBOX_STATUS_INTERNAL_ERROR";
    }
}

std::string_view GetSignalMessage(int sig)
{
    const static std::unordered_map<int, std::string_view> signalMap = {
        {SIGHUP, "Hangup"},
        {SIGINT, "Interrupt by user input"},
        {SIGQUIT, "Quit"},
        {SIGILL, "Illegal instruction"},
        {SIGTRAP, "Trace/breakpoint trap"},
        {SIGABRT, "Aborted"},
        {SIGBUS, "Bus error"},
        {SIGFPE, "Floating point exception"},
        // The program was killed by the watcher
        {SIGKILL, "Killed by watcher: Program ran timed out"},
        // May be caused by the program exceeding the memory limit
        {SIGSEGV, "Segmentation fault: Access to illegal memory or memory overruns"},
        {SIGPIPE, "Broken pipe"},
        {SIGALRM, "Alarm clock"},
        {SIGTERM, "Terminated"},
        {SIGSTKFLT, "Stack fault"},
        {SIGCHLD, "Child exited"},
        {SIGCONT, "Continued"},
        {SIGSTOP, "Stopped (signal)"},
        {SIGTSTP, "Stopped"},
        {SIGTTIN, "Stopped (tty input)"},
        {SIGTTOU, "Stopped (tty output)"},
        {SIGURG, "Urgent I/O condition"},
        {SIGXCPU, "CPU time limit exceeded"},
        {SIGXFSZ, "File size limit exceeded"},
        {SIGVTALRM, "Virtual timer expired"},
        {SIGPROF, "Profiling timer expired"},
        {SIGIO, "I/O possible"},
        {SIGPWR, "Power failure"},
        {SIGSYS, "Bad system call"},
    };

    if (signalMap.count(sig))
    {
        return signalMap.at(sig);
    }
    return "OK";
}

void PrintResult(const SandboxResult &result)
{
    std::cout << "============================= Result =============================" << std::endl;
    std::cout << "ExitCode: " << result.ExitCode << std::endl;
    std::cout << "Status: " << GetStatusName(static_cast<SandboxStatus>(result.Status)) << std::endl;
    std::cout << "Memory: " << result.MemoryUsage << std::endl;
    std::cout << "RealTime: " << result.RealTimeUsage << std::endl;
    std::cout << "UserTime: " << result.CpuTimeUsage << std::endl;
    std::cout << "Signal: " << result.Signal << " " << std::quoted(GetSignalMessage(result.Signal)) << std::endl;
    std::cout << "===================================================================" << std::endl;
}

TEST(SandboxTest, ExpectedAccepted)
{
    INIT_SANDBOX_TESTCASE(ExpectedAccepted);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.ExitCode, 0);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_SUCCESS);
}

TEST(SandboxTest, ExpectedRealTimeout)
{
    INIT_SANDBOX_TESTCASE(ExpectedTimeout);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.ExitCode, 0);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED);
}

TEST(SandboxTest, ExpectedCpuTimeout)
{
    INIT_SANDBOX_TESTCASE(ExpectedCpuTimeout);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.ExitCode, 0);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED);
}

TEST(SandboxTest, ExpectedMemoryLimitExceeded)
{
    INIT_SANDBOX_TESTCASE(ExpectedMemoryLimitExceeded);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.ExitCode, 0);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED);
}

TEST(SandboxTest, ExpectedRuntimeError)
{
    INIT_SANDBOX_TESTCASE(ExpectedRuntimeError);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_RUNTIME_ERROR);
}

TEST(SandboxTest, ExpectedOutputLimitExceeded)
{
    INIT_SANDBOX_TESTCASE(ExpectedOutputLimitExceeded);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.Signal, SIGXFSZ);
}

TEST(SandboxTest, ExpectedProcessLimitExceeded)
{
    INIT_SANDBOX_TESTCASE(ExpectedProcessLimitExceeded);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_RUNTIME_ERROR);
}

TEST(SandboxTest, ExpectedKilledBySecomp)
{
    INIT_SANDBOX_TESTCASE(ExpectedKilledBySecomp);
    ASSERT_EQ(StartSandbox(&configuration, &result), SANDBOX_STATUS_SUCCESS);
    PrintResult(result);
    ASSERT_EQ(result.Status, SANDBOX_STATUS_RUNTIME_ERROR);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}