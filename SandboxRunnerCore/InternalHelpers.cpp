#include "InternalHelpers.h"
#include "Sandbox.h"
#include <sstream>
#include <algorithm>

extern char **environ;

namespace SandboxInternal
{

InternalConfig InternalConfig::FromCConfig(const ::SandboxConfiguration *config)
{
    InternalConfig internal;

    internal.TaskName = config->TaskName ? config->TaskName : "";
    internal.UserCommand = config->UserCommand ? config->UserCommand : "";
    internal.WorkingDirectory = config->WorkingDirectory ? config->WorkingDirectory : "";

    internal.InputFile = config->InputFile ? config->InputFile : "";
    internal.OutputFile = config->OutputFile ? config->OutputFile : "";
    internal.ErrorFile = config->ErrorFile ? config->ErrorFile : "";
    internal.LogFile = config->LogFile ? config->LogFile : "";

    internal.MaxMemoryToCrash = config->MaxMemoryToCrash;
    internal.MaxMemory = config->MaxMemory;
    internal.MaxStack = config->MaxStack;
    internal.MaxCpuTime = config->MaxCpuTime;
    internal.MaxRealTime = config->MaxRealTime;
    internal.MaxOutputSize = config->MaxOutputSize;
    internal.MaxProcessCount = config->MaxProcessCount;
    internal.Policy = config->Policy;

    return internal;
}

std::vector<std::string> InternalConfig::ParseCommandArgs() const
{
    std::vector<std::string> args;
    std::istringstream iss(UserCommand);
    std::string token;

    while (iss >> token)
    {
        args.push_back(std::move(token));
    }

    return args;
}

} // namespace SandboxInternal
