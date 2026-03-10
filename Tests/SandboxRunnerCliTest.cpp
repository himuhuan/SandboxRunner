#include "SandboxTest.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#ifndef _WIN32
#include <sys/wait.h>
#endif

namespace
{

struct CommandResult
{
    int ExitCode;
    std::string StdOut;
    std::string StdErr;
};

std::filesystem::path ResolveSandboxRunnerPath()
{
    const auto currentDirectory = std::filesystem::current_path();
    const std::vector<std::filesystem::path> candidates = {
        currentDirectory / ".." / "SandboxRunner" / "SandboxRunner",
        currentDirectory / "SandboxRunner" / "SandboxRunner",
#ifdef _WIN32
        currentDirectory / ".." / "SandboxRunner" / "SandboxRunner.exe",
        currentDirectory / "SandboxRunner" / "SandboxRunner.exe",
#endif
    };

    for (const auto &candidate : candidates)
    {
        std::error_code errorCode;
        if (std::filesystem::exists(candidate, errorCode))
        {
            return std::filesystem::weakly_canonical(candidate, errorCode);
        }
    }

    return {};
}

std::string ReadTextFile(const std::filesystem::path &path)
{
    std::ifstream input(path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::filesystem::path MakeTemporaryPath(std::string_view suffix)
{
    static std::mt19937_64 generator(std::random_device{}());
    const auto randomValue = generator();
    return std::filesystem::temp_directory_path() /
           ("sandboxrunner-cli-" + std::to_string(randomValue) + "-" + std::string(suffix));
}

#ifdef _WIN32
std::string QuoteShellArgument(const std::string &argument)
{
    std::string escaped = "\"";
    for (const char c : argument)
    {
        if (c == '"')
        {
            escaped += "\\\"";
            continue;
        }
        escaped += c;
    }
    escaped += "\"";
    return escaped;
}
#else
std::string QuoteShellArgument(const std::string &argument)
{
    std::string escaped = "'";
    for (const char c : argument)
    {
        if (c == '\'')
        {
            escaped += "'\\''";
            continue;
        }
        escaped += c;
    }
    escaped += "'";
    return escaped;
}
#endif

int DecodeExitCode(int systemResult)
{
#ifdef _WIN32
    return systemResult;
#else
    if (WIFEXITED(systemResult))
    {
        return WEXITSTATUS(systemResult);
    }
    if (WIFSIGNALED(systemResult))
    {
        return 128 + WTERMSIG(systemResult);
    }
    return systemResult;
#endif
}

CommandResult RunSandboxRunner(const std::vector<std::string> &arguments)
{
    const auto executablePath = ResolveSandboxRunnerPath();
    EXPECT_FALSE(executablePath.empty()) << "SandboxRunner executable not found";

    const auto stdoutPath = MakeTemporaryPath("stdout.txt");
    const auto stderrPath = MakeTemporaryPath("stderr.txt");

    std::string command = QuoteShellArgument(executablePath.string());
    for (const auto &argument : arguments)
    {
        command += " " + QuoteShellArgument(argument);
    }

#ifdef _WIN32
    command += " > " + QuoteShellArgument(stdoutPath.string()) + " 2> " + QuoteShellArgument(stderrPath.string());
#else
    command += " > " + QuoteShellArgument(stdoutPath.string()) + " 2> " + QuoteShellArgument(stderrPath.string());
#endif

    const int systemResult = std::system(command.c_str());

    CommandResult result{
        .ExitCode = DecodeExitCode(systemResult),
        .StdOut = ReadTextFile(stdoutPath),
        .StdErr = ReadTextFile(stderrPath),
    };

    std::error_code errorCode;
    std::filesystem::remove(stdoutPath, errorCode);
    errorCode.clear();
    std::filesystem::remove(stderrPath, errorCode);
    return result;
}

} // namespace

TEST(SandboxRunnerCliTest, OutputsJsonResult)
{
#ifdef __linux__
    const auto result = RunSandboxRunner({"--format", "json", "/bin/true"});
    ASSERT_EQ(result.ExitCode, 0) << result.StdErr;

    const auto json = nlohmann::json::parse(result.StdOut);
    EXPECT_EQ(json.at("Status"), SANDBOX_STATUS_SUCCESS);
    EXPECT_EQ(json.at("StatusName"), "SUCCESS");
    EXPECT_EQ(json.at("ExitCode"), 0);
    EXPECT_EQ(json.at("Signal"), 0);
    EXPECT_TRUE(json.contains("CpuTimeUsage"));
    EXPECT_TRUE(json.contains("RealTimeUsage"));
    EXPECT_TRUE(json.contains("MemoryUsage"));
#else
    GTEST_SKIP() << "CLI sandbox execution test is only supported on Linux.";
#endif
}

TEST(SandboxRunnerCliTest, OutputsTextResultAndRejectsInvalidFormat)
{
#ifdef __linux__
    const auto textResult = RunSandboxRunner({"--format", "text", "/bin/true"});
    ASSERT_EQ(textResult.ExitCode, 0) << textResult.StdErr;
    EXPECT_NE(textResult.StdOut.find("Status:       SUCCESS"), std::string::npos);
    EXPECT_NE(textResult.StdOut.find("ExitCode:     0"), std::string::npos);
    EXPECT_NE(textResult.StdOut.find("Signal:       0"), std::string::npos);

    const auto invalidFormatResult = RunSandboxRunner({"--format", "xml", "/bin/true"});
    ASSERT_EQ(invalidFormatResult.ExitCode, 1);
    EXPECT_NE(invalidFormatResult.StdErr.find("Invalid output format: xml"), std::string::npos);
    EXPECT_NE(invalidFormatResult.StdErr.find("Supported formats: json, text"), std::string::npos);
#else
    GTEST_SKIP() << "CLI sandbox execution test is only supported on Linux.";
#endif
}
