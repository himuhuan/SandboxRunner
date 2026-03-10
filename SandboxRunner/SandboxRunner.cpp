#include "SandboxRunner.h"
#include "../SandboxRunnerCore/Sandbox.h"
#include "cmdline.h"
#include "stduuid/uuid.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <nlohmann/json.hpp>

struct CliOptions
{
    SandboxConfiguration Configuration;
    std::string Format;
};

CliOptions GetCliOptions(int argc, char **argv);

namespace
{

const char *GetStatusName(int status)
{
    switch (status)
    {
    case SANDBOX_STATUS_SUCCESS:
        return "SUCCESS";
    case SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED:
        return "MEMORY_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_RUNTIME_ERROR:
        return "RUNTIME_ERROR";
    case SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED:
        return "CPU_TIME_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED:
        return "REAL_TIME_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_PROCESS_LIMIT_EXCEEDED:
        return "PROCESS_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_OUTPUT_LIMIT_EXCEEDED:
        return "OUTPUT_LIMIT_EXCEEDED";
    case SANDBOX_STATUS_ILLEGAL_OPERATION:
        return "ILLEGAL_OPERATION";
    default:
        return "INTERNAL_ERROR";
    }
}

std::string NormalizeFormat(std::string format)
{
    std::transform(format.begin(), format.end(), format.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return format;
}

void PrintResultAsJson(const SandboxResult &result)
{
    nlohmann::json j;
    j["Status"]        = result.Status;
    j["StatusName"]    = GetStatusName(result.Status);
    j["ExitCode"]      = result.ExitCode;
    j["Signal"]        = result.Signal;
    j["CpuTimeUsage"]  = result.CpuTimeUsage;
    j["RealTimeUsage"] = result.RealTimeUsage;
    j["MemoryUsage"]   = result.MemoryUsage;
    std::cout << j.dump() << std::endl;
}

void PrintResultAsText(const SandboxResult &result)
{
    std::cout << "Status:       " << GetStatusName(result.Status) << std::endl;
    std::cout << "ExitCode:     " << result.ExitCode << std::endl;
    std::cout << "Signal:       " << result.Signal << std::endl;
    std::cout << "CpuTimeUsage: " << result.CpuTimeUsage << " ms" << std::endl;
    std::cout << "RealTimeUsage:" << result.RealTimeUsage << " ms" << std::endl;
    std::cout << "MemoryUsage:  " << result.MemoryUsage << " bytes" << std::endl;
}

char *CopyString(const std::string &s)
{
    if (s.empty())
        return nullptr;

    return strdup(s.c_str());
}

} // namespace

int main(int argc, char *argv[])
{
    auto [configuration, format] = GetCliOptions(argc, argv);
    SandboxResult result{};

    int infraStatus = StartSandbox(&configuration, &result);

    if (infraStatus != SANDBOX_STATUS_SUCCESS && result.Status == 0)
    {
        result.Status = infraStatus;
        fprintf(stderr, "Failed to start sandbox\n");
    }

    if (format == "json")
        PrintResultAsJson(result);
    else
        PrintResultAsText(result);

    return (infraStatus == SANDBOX_STATUS_SUCCESS) ? 0 : infraStatus;
}

CliOptions GetCliOptions(int argc, char **argv)
{
    cmdline::parser parser;
    parser.add<std::string>("name", 'n', "The name of the task", false);
    parser.add<std::string>("dir", 'd', "The working directory of the task", false, ".");
    parser.add<std::string>("input", 'i', "The input file of the task", false);
    parser.add<std::string>("output", 'o', "The output file of the task", false);
    parser.add<std::string>("error", 'e', "The error file of the task", false);
    parser.add<std::string>("log", 'l', "The log file of the task", false);
    parser.add<uint64_t>("memory", 0, "Memory limit of the task", false, 0);
    parser.add<uint64_t>("stack", 0, "Stack limit of the task", false, 0);
    parser.add<uint64_t>("cpu", 0, "CPU time limit of the task", false, 0);
    parser.add<uint64_t>("real", 0, "Real time limit of the task", false, 0);
    parser.add<int>("process", 0, "Process count limit of the task (-1 means unlimited)", false, -1);
    parser.add<uint64_t>("output-size", 0, "Output size limit of the task", false, 0);
    parser.add<std::string>("policy", 'p', "The policy name of the task", false, "default");
    parser.add<std::string>("format", 'f', "Output format (json or text)", false, "json");
    parser.footer("program [args...]");

    parser.parse(argc, argv);

    SandboxConfiguration configuration{};

    // do not worry about memory leak --- always runs once and exit.

    configuration.TaskName = CopyString(parser.get<std::string>("name"));
    if (configuration.TaskName == nullptr)
    {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        std::string randomName = uuids::to_string(uuids::uuid_random_generator{generator}());
        configuration.TaskName = CopyString(randomName);
    }

    configuration.WorkingDirectory = CopyString(parser.get<std::string>("dir"));
    configuration.InputFile        = CopyString(parser.get<std::string>("input"));
    configuration.OutputFile       = CopyString(parser.get<std::string>("output"));
    configuration.LogFile          = CopyString(parser.get<std::string>("log"));
    configuration.ErrorFile        = CopyString(parser.get<std::string>("error"));
    configuration.MaxMemory        = parser.get<uint64_t>("memory");
    configuration.MaxStack         = parser.get<uint64_t>("stack");
    configuration.MaxCpuTime       = parser.get<uint64_t>("cpu");
    configuration.MaxRealTime      = parser.get<uint64_t>("real");
    configuration.MaxProcessCount  = parser.get<int>("process");
    configuration.MaxOutputSize    = parser.get<uint64_t>("output-size");
    configuration.Policy           = CopyString(parser.get<std::string>("policy"));

    if (parser.rest().empty())
    {
        fprintf(stderr, "No command specified\n");
        fprintf(stderr, "%s", parser.usage().c_str());
        exit(1);
    }
    else
    {
        configuration.UserCommand = CopyString(parser.rest()[0]);
    }

    std::string format = NormalizeFormat(parser.get<std::string>("format"));
    constexpr std::array<const char *, 2> kSupportedFormats = {"json", "text"};
    const bool isSupportedFormat = std::any_of(kSupportedFormats.begin(), kSupportedFormats.end(),
                                               [&format](const char *supportedFormat) {
                                                   return format == supportedFormat;
                                               });
    if (!isSupportedFormat)
    {
        fprintf(stderr, "Invalid output format: %s\n", format.c_str());
        fprintf(stderr, "Supported formats: json, text\n");
        exit(1);
    }

    return {configuration, format};
}
