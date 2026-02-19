#include <chrono>
#include "SandboxImpl.h"
#include "fmt/core.h"
#include "../Logger.h"
#include "ErrorHandler.h"

#include "../SandboxUtils.h"
#include "../InternalHelpers.h"
#include "SandboxChildProcess.h"
#include "SandboxMonitor.h"

#include <sys/wait.h>
#include <sys/resource.h>

constexpr int MAX_ARGUMENTS       = 128;
constexpr int USER_COMMAND_LENGTH = 1024;

SandboxImpl::SandboxImpl(const SandboxConfiguration *config, SandboxResult &result) : _config(config), _result(result)
{
    memset(&result, 0, sizeof(SandboxResult));
}

int SandboxImpl::Run()
{
    using SandboxInternal::ErrorContext;
    using SandboxInternal::InternalError;
    using SandboxInternal::HandleParentError;

    Logger::Initialize(_config->TaskName, _config->LogFile, Logger::LoggerLevel::Debug);
    Logger::Info("Start running sandboxed process");

    /* No permission */
    /*if (getuid() != 0)
    {
        Logger::Error("No permission to run sandboxed process");
        return SANDBOX_STATUS_INTERNAL_ERROR;
    }*/

    // Convert C configuration to internal modern C++ representation
    auto internalConfig = SandboxInternal::InternalConfig::FromCConfig(_config);

    // Parse command into arguments using modern C++ string handling
    auto cmdArgs = internalConfig.ParseCommandArgs();
    if (cmdArgs.empty() || cmdArgs.size() >= MAX_ARGUMENTS)
    {
        return HandleParentError(ErrorContext(InternalError::InvalidCommandArgs, "Invalid argument count"));
    }

    // Convert to char* array for execve
    std::vector<char *> args;
    args.reserve(cmdArgs.size() + 1);
    for (auto &arg : cmdArgs)
    {
        args.push_back(const_cast<char *>(arg.c_str()));
    }
    args.push_back(nullptr);

    Logger::Info("Starting sandboxed process: \"{0}\"", _config->UserCommand);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    pid_t sandboxPid = fork();
    if (sandboxPid < 0)
    {
        return HandleParentError(ErrorContext(InternalError::ForkFailed, "Failed to fork process"));
    }

    if (sandboxPid == 0) /* Child Process */
    {
        RunSandboxProcess(args[0], args.data(), _config);
    }
    else
    {
        /* Parent Process */

        SandboxInternal::ScopedThread monitorThread;
        SandboxMonitorConfiguration monitorConfig{.Timeout = _config->MaxRealTime, .Pid = sandboxPid};

        if (_config->MaxRealTime != UNLIMITED)
        {
            pthread_t threadId = 0;
            if (StartSandboxMonitor(&threadId, &monitorConfig) != 0)
            {
                return HandleParentError(ErrorContext(InternalError::MonitorThreadStartFailed, "Failed to start monitor thread"));
            }
            monitorThread.reset(threadId);
        }

        int childStatus;
        rusage usage = {};
        if (wait4(sandboxPid, &childStatus, 0, &usage) == -1)
        {
            kill(sandboxPid, SIGKILL);
            return HandleParentError(ErrorContext(InternalError::WaitFailed, "Failed to wait for child process"));
        }

        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        _result.RealTimeUsage = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // ScopedThread destructor will automatically clean up the monitor thread

        /* terminated by a signal */
        if (WIFSIGNALED(childStatus))
            _result.Signal = WTERMSIG(childStatus);

        if (_result.Signal == SIGUSR1)
        {
            Logger::Error("An internal error occurred in the sandboxed process, terminated!");
            _result.Status = SANDBOX_STATUS_INTERNAL_ERROR;
            return SANDBOX_STATUS_INTERNAL_ERROR;
        }

        _result.ExitCode     = WEXITSTATUS(childStatus);
        _result.MemoryUsage  = usage.ru_maxrss * 1024;
        _result.CpuTimeUsage = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;

        if (_result.ExitCode != 0 || _result.Signal != 0)
        {
            if (_result.Signal == SIGSEGV && _config->MaxMemory != UNLIMITED
                && _result.MemoryUsage > _config->MaxMemory)
                _result.Status = SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED;
            else if (_result.Signal == SIGKILL && _config->MaxRealTime != UNLIMITED
                     && _result.RealTimeUsage >= _config->MaxRealTime)
                _result.Status = SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED;
            else
                _result.Status = (_result.Signal == SIGSYS) ? SANDBOX_STATUS_ILLEGAL_OPERATION : SANDBOX_STATUS_RUNTIME_ERROR;
        }

        if (_config->MaxMemory != UNLIMITED && _result.MemoryUsage >= _config->MaxMemory)
            _result.Status = SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED;
        else if (_config->MaxCpuTime != UNLIMITED && _result.CpuTimeUsage >= _config->MaxCpuTime)
            _result.Status = SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED;
    }
    return 0;
}
