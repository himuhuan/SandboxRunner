#include "SandboxChildProcess.h"
#include "SandboxImpl.h"
#include "../Logger.h"
#include "../InternalHelpers.h"
#include "../Policy/ResourceConfig.h"
#include "SecurePolicy.h"
#include "ErrorHandler.h"
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

char *const *GetEnvironmentVariables(const SandboxConfiguration *configuration)
{
    if (configuration->EnvironmentVariables == nullptr || configuration->EnvironmentVariablesCount == 0)
        return environ;

    return const_cast<char *const *>(configuration->EnvironmentVariables);
}

void RunSandboxProcess(const char *programPath, char *const *programArgs, const SandboxConfiguration *configuration)
{
    using SandboxInternal::UniqueFile;
    using SandboxInternal::ErrorContext;
    using SandboxInternal::InternalError;
    using SandboxInternal::HandleChildError;

    UniqueFile inputStream;
    UniqueFile outputStream;
    UniqueFile errorStream;

    const auto resourceConfig = SandboxPolicyEngine::ResourceConfig::FromCConfig(*configuration);
    const auto cpuLimit = static_cast<rlim_t>(resourceConfig.GetEffectiveCpuLimitSeconds());

    if (configuration->WorkingDirectory && chdir(configuration->WorkingDirectory) != 0)
        HandleChildError(ErrorContext(InternalError::InvalidWorkingDirectory, "Failed to switch working directory"));

    const auto maxMemoryToCrash = static_cast<rlim_t>(resourceConfig.GetEffectiveMaxMemoryToCrash());
    Logger::Debug("Applying job limits");
    if (!SetResourceLimit(RLIMIT_AS, maxMemoryToCrash)
        || !SetResourceLimit(RLIMIT_STACK, static_cast<rlim_t>(resourceConfig.MaxStack))
        || !SetResourceLimit(RLIMIT_CPU, cpuLimit)
        || (resourceConfig.MaxProcessCount >= 0
            && !SetResourceLimit(RLIMIT_NPROC, static_cast<rlim_t>(resourceConfig.MaxProcessCount)))
        || !SetResourceLimit(RLIMIT_FSIZE, static_cast<rlim_t>(resourceConfig.MaxOutputSize)))
    {
        HandleChildError(ErrorContext(InternalError::ResourceLimitFailed, "Failed to apply job limits"));
    }

    if (configuration->InputFile)
    {
        inputStream.reset(fopen(configuration->InputFile, "r"));
        if (!inputStream)
            HandleChildError(ErrorContext(InternalError::InputFileOpenFailed, "Failed to open input file"));
        if (dup2(fileno(inputStream.get()), fileno(stdin)) == -1)
            HandleChildError(ErrorContext(InternalError::FileRedirectFailed, "Failed to redirect input file"));
    }

    if (configuration->OutputFile)
    {
        outputStream.reset(fopen(configuration->OutputFile, "w"));
        if (!outputStream)
            HandleChildError(ErrorContext(InternalError::OutputFileOpenFailed, "Failed to open output file"));
        if (dup2(fileno(outputStream.get()), fileno(stdout)) == -1)
            HandleChildError(ErrorContext(InternalError::FileRedirectFailed, "Failed to redirect output file"));
    }

    if (configuration->ErrorFile)
    {
        if (outputStream && strcmp(configuration->OutputFile, configuration->ErrorFile) == 0)
        {
            Logger::Info("Same path for output and error file");
            // Share the same FILE* - no need to open again
            if (dup2(fileno(outputStream.get()), fileno(stderr)) == -1)
                HandleChildError(ErrorContext(InternalError::FileRedirectFailed, "Failed to redirect error file"));
        }
        else
        {
            errorStream.reset(fopen(configuration->ErrorFile, "w"));
            if (!errorStream)
                HandleChildError(ErrorContext(InternalError::ErrorFileOpenFailed, "Failed to open error file"));

            if (dup2(fileno(errorStream.get()), fileno(stderr)) == -1)
                HandleChildError(ErrorContext(InternalError::FileRedirectFailed, "Failed to redirect error file"));
        }
    }

    if (configuration->Policy != DEFAULT)
        Logger::Info("Applying custom rules: {0}", configuration->Policy);

    if (ApplyLinuxSecurePolicy(programPath, configuration))
    {
        Logger::Info("Applied policy to {0}, start running the sandboxed process", programPath);
    }
    else
    {
        HandleChildError(ErrorContext(InternalError::PolicyApplicationFailed, "Failed to apply policy"));
    }

    execve(programPath, programArgs, GetEnvironmentVariables(configuration));
    HandleChildError(ErrorContext(InternalError::ExecFailed, "Failed to execute the user command"));
}
