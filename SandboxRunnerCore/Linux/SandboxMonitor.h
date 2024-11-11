#ifndef SANDBOX_MONITOR_H
#define SANDBOX_MONITOR_H

#include <cstdint>
#include <optional>
#include <pthread.h>

struct SandboxMonitorConfiguration
{
    uint64_t Timeout; // in milliseconds
    pid_t Pid; // process to monitor
};

/**
 * @brief Create a thread to monitor the running time of the program,
 * kill the program if it exceeds the limit
 */
int StartSandboxMonitor(pthread_t *thread, const SandboxMonitorConfiguration *configuration);

#endif //! SANDBOX_MONITOR_H
