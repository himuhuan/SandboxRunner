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

    if (config->EnvironmentVariables && config->EnvironmentVariablesCount > 0)
    {
        internal.EnvironmentVariables.reserve(config->EnvironmentVariablesCount);
        for (uint16_t i = 0; i < config->EnvironmentVariablesCount; ++i)
        {
            if (config->EnvironmentVariables[i])
            {
                internal.EnvironmentVariables.emplace_back(config->EnvironmentVariables[i]);
            }
        }
    }

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

std::vector<char *> InternalConfig::GetEnvPointers() const
{
    std::vector<char *> envPtrs;

    if (EnvironmentVariables.empty())
    {
        // Use system environment
        for (char **env = environ; *env != nullptr; ++env)
        {
            envPtrs.push_back(*env);
        }
    }
    else
    {
        // Use custom environment (const_cast is safe here as execve doesn't modify)
        for (const auto &envVar : EnvironmentVariables)
        {
            envPtrs.push_back(const_cast<char *>(envVar.c_str()));
        }
    }

    envPtrs.push_back(nullptr);
    return envPtrs;
}

} // namespace SandboxInternal
