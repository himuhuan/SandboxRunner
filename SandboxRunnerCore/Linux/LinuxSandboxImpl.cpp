#include <chrono>
#include "SandboxImpl.h"
#include "fmt/core.h"
#include "../Logger.h"

#include "../SandboxUtils.h"
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
    Logger::Initialize(_config->TaskName, _config->LogFile, Logger::LoggerLevel::Debug);
    Logger::Info("Start running sandboxed process");

    /* No permission */
    /*if (getuid() != 0)
    {
        Logger::Error("No permission to run sandboxed process");
        return SANDBOX_STATUS_INTERNAL_ERROR;
    }*/

    // In some cases, the user command may be modified, so we need to copy it to a new buffer
    char userCommand[USER_COMMAND_LENGTH] = {};
    strncpy(userCommand, _config->UserCommand, USER_COMMAND_LENGTH);

    char *args[MAX_ARGUMENTS];
    int argc;
    if ((argc = SplitString(userCommand, " ", args, MAX_ARGUMENTS)) == MAX_ARGUMENTS)
    {
        Logger::Error("Failed to split user command, too many arguments");
        return SANDBOX_STATUS_INTERNAL_ERROR;
    }
    args[argc] = nullptr;

    Logger::Info("Starting sandboxed process: \"{0}\"", _config->UserCommand);
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    pid_t sandboxPid = fork();
    if (sandboxPid < 0)
    {
        Logger::Error("Failed to fork process");
        exit(SANDBOX_STATUS_INTERNAL_ERROR);
    }

    if (sandboxPid == 0) /* Child Process */
    {
        RunSandboxProcess(args[0], args, _config);
    }
    else
    {
        /* Parent Process */

        pthread_t monitorThreadId = 0;
        SandboxMonitorConfiguration monitorConfig{.Timeout = _config->MaxRealTime, .Pid = sandboxPid};

        if (_config->MaxRealTime != UNLIMITED)
        {
            if (StartSandboxMonitor(&monitorThreadId, &monitorConfig) != 0)
            {
                Logger::Error("Failed to start monitor thread");
                return SANDBOX_STATUS_INTERNAL_ERROR;
            }
        }

        int childStatus;
        rusage usage = {};
        if (wait4(sandboxPid, &childStatus, 0, &usage) == -1)
        {
            kill(sandboxPid, SIGKILL);
            Logger::Error("Failed to wait for child process");
            return SANDBOX_STATUS_INTERNAL_ERROR;
        }

        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        _result.RealTimeUsage = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        /* May required cleanup */
        if (monitorThreadId != 0)
        {
            pthread_cancel(monitorThreadId);
            pthread_join(monitorThreadId, nullptr);
        }

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
                _result.Status = SANDBOX_STATUS_RUNTIME_ERROR;
        }

        if (_config->MaxMemory != UNLIMITED && _result.MemoryUsage >= _config->MaxMemory)
            _result.Status = SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED;
        else if (_config->MaxCpuTime != UNLIMITED && _result.CpuTimeUsage >= _config->MaxCpuTime)
            _result.Status = SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED;
    }
    return 0;
}
