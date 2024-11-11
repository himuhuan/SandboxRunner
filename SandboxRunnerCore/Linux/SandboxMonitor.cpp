#include "SandboxMonitor.h"

#include "../Logger.h"

#include <unistd.h>
#include <csignal>
#include <optional>

namespace
{
void *MonitorThread(void *arg)
{
    auto *config = static_cast<SandboxMonitorConfiguration *>(arg);
    auto pid = config->Pid;
    auto timeout = (config->Timeout + 999) / 1000;

    auto sleepTime = sleep(timeout);
    if (kill(pid, 0) == 0)
    {
        Logger::Info("Program (pid @{}) killed: timeout after {}s", pid, timeout - sleepTime);
        kill(pid, SIGKILL);
    }
    pthread_exit(nullptr);

    // ReSharper disable once CppDFAUnreachableCode
    return nullptr;
}
} // namespace

int StartSandboxMonitor(pthread_t *thread, const SandboxMonitorConfiguration *configuration)
{
    return pthread_create(thread, nullptr, MonitorThread, const_cast<SandboxMonitorConfiguration *>(configuration));
}