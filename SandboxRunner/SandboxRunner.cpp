#include "SandboxRunner.h"
#include "../SandboxRunnerCore/Sandbox.h"
#include "cmdline.h"
#include "stduuid/uuid.h"

SandboxConfiguration GetSandboxConfiguration(int argc, char **argv);

int main(int argc, char *argv[])
{
    SandboxConfiguration configuration = GetSandboxConfiguration(argc, argv);
    SandboxResult result{};
    
    if (int status = StartSandbox(&configuration, &result); status != SANDBOX_STATUS_SUCCESS)
    {
        fprintf(stderr, "Failed to start sandbox\n");
        return status;
    }
    return 0;
}

namespace
{

char *CopyString(const std::string &s)
{
    if (s.empty())
        return nullptr;

    return strdup(s.c_str());
}

} // namespace

SandboxConfiguration GetSandboxConfiguration(int argc, char **argv)
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
    parser.add<uint64_t>("process", 0, "Process count limit of the task", false, 0);
    parser.add<uint64_t>("output-size", 0, "Output size limit of the task", false, 0);
    parser.add<std::string>("policy", 'p', "The policy of the task", false, "default",
                            cmdline::oneof<std::string>("default", "cxx"));
    parser.footer("program [args...]");

    parser.parse(argc, argv);

    SandboxConfiguration configuration{};

    // do not worry about memory leak --- always runs once and exit.

    configuration.TaskName = CopyString(parser.get<std::string>("name"));
    if (configuration.TaskName == nullptr)
    {
        std::string randomName = to_string(uuids::uuid_system_generator{}());
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

    return configuration;
}
