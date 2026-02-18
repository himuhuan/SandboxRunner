#include "SandboxChildProcess.h"
#include "SandboxImpl.h"
#include "../Logger.h"
#include "SecurePolicy.h"
#include <csignal>
#include <sched.h>
#include <sys/resource.h>
#include <unistd.h>

extern char **environ;

bool SetResourceLimit(const int resource, rlim_t val)
{
    if (resource != RLIMIT_NPROC && val == UNLIMITED)
        return true;
    const rlimit limit{.rlim_cur = val, .rlim_max = val};
    return setrlimit(resource, &limit) == 0;
}

rlim_t GetEffectiveMaxMemoryToCrash(const SandboxConfiguration *configuration)
{
    if (configuration->MaxMemoryToCrash != 0
        && (configuration->MaxMemory == UNLIMITED || configuration->MaxMemoryToCrash >= configuration->MaxMemory))
        return configuration->MaxMemoryToCrash;

    if (configuration->MaxMemory == UNLIMITED)
        return UNLIMITED;

    return configuration->MaxMemory * 2;
}

char *const *GetEnvironmentVariables(const SandboxConfiguration *configuration)
{
    if (configuration->EnvironmentVariables == nullptr || configuration->EnvironmentVariablesCount == 0)
        return environ;

    return const_cast<char *const *>(configuration->EnvironmentVariables);
}

#define CHILD_FATAL_ERROR(message, code)                                                                               \
    {                                                                                                                  \
        Logger::Error("(CHILD {0}): {1}, error: {2}", (int)getpid(), message, strerror(errno));                        \
        if (inputStream != nullptr)                                                                                    \
            fclose(inputStream);                                                                                       \
        if (outputStream != nullptr)                                                                                   \
            fclose(outputStream);                                                                                      \
        if (errorStream != nullptr && outputStream != errorStream)                                                     \
            fclose(errorStream);                                                                                       \
        raise(SIGUSR1);                                                                                                \
        exit(code);                                                                                                    \
    }

void RunSandboxProcess(const char *programPath, char *const *programArgs, const SandboxConfiguration *configuration)
{
    FILE *inputStream = nullptr, *outputStream = nullptr, *errorStream = nullptr;

    rlim_t cpuLimit = configuration->MaxCpuTime != 0 ? (configuration->MaxCpuTime + 1000) / 1000 : UNLIMITED;

    if (configuration->WorkingDirectory && chdir(configuration->WorkingDirectory) != 0)
        CHILD_FATAL_ERROR("Failed to switch working directory", SANDBOX_STATUS_INTERNAL_ERROR);

    const rlim_t maxMemoryToCrash = GetEffectiveMaxMemoryToCrash(configuration);
    Logger::Debug("Applying job limits");
    if (!SetResourceLimit(RLIMIT_AS, maxMemoryToCrash)
        || !SetResourceLimit(RLIMIT_STACK, configuration->MaxStack) || !SetResourceLimit(RLIMIT_CPU, cpuLimit)
        || (configuration->MaxProcessCount >= 0 && !SetResourceLimit(RLIMIT_NPROC, configuration->MaxProcessCount))
        || !SetResourceLimit(RLIMIT_FSIZE, configuration->MaxOutputSize))
    {
        CHILD_FATAL_ERROR("Failed to apply job limits", SANDBOX_STATUS_INTERNAL_ERROR);
    }

    if (configuration->InputFile)
    {
        inputStream = fopen(configuration->InputFile, "r");
        if (inputStream == nullptr)
            CHILD_FATAL_ERROR("Failed to open input file", SANDBOX_STATUS_INTERNAL_ERROR);
        if (dup2(fileno(inputStream), fileno(stdin)) == -1)
            CHILD_FATAL_ERROR("Failed to redirect input file", SANDBOX_STATUS_INTERNAL_ERROR);
    }

    if (configuration->OutputFile)
    {
        outputStream = fopen(configuration->OutputFile, "w");
        if (outputStream == nullptr)
            CHILD_FATAL_ERROR("Failed to open output file", SANDBOX_STATUS_INTERNAL_ERROR);
        if (dup2(fileno(outputStream), fileno(stdout)) == -1)
            CHILD_FATAL_ERROR("Failed to redirect output file", SANDBOX_STATUS_INTERNAL_ERROR);
    }

    if (configuration->ErrorFile)
    {
        if (outputStream != nullptr && strcmp(configuration->OutputFile, configuration->ErrorFile) == 0)
        {
            Logger::Info("Same path for output and error file");
            errorStream = outputStream;
        }
        else
        {
            errorStream = fopen(configuration->ErrorFile, "w");
            if (errorStream == nullptr)
                CHILD_FATAL_ERROR("Failed to open error file", SANDBOX_STATUS_INTERNAL_ERROR);
        }

        if (dup2(fileno(errorStream), fileno(stderr)) == -1)
            CHILD_FATAL_ERROR("Failed to redirect error file", SANDBOX_STATUS_INTERNAL_ERROR);
    }

    if (configuration->Policy != DEFAULT)
        Logger::Info("Applying custom rules: {0}", configuration->Policy);

    if (ApplyLinuxSecurePolicy(programPath, configuration))
    {
        Logger::Info("Applied policy to {0}, start running the sandboxed process", programPath);
    }
    else 
    {
        CHILD_FATAL_ERROR("Failed to apply policy", SANDBOX_STATUS_INTERNAL_ERROR);
    }

    execve(programPath, programArgs, GetEnvironmentVariables(configuration));
    CHILD_FATAL_ERROR("Failed to execute the user command", SANDBOX_STATUS_INTERNAL_ERROR);
}
