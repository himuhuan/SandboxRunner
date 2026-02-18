#pragma once

#ifndef SANDBOX_LIB_INC
#define SANDBOX_LIB_INC

#include <cstdint>
#include <memory>

// 1.3.2
constexpr int SANDBOX_VERSION = 0x010302;

#define MAJOR_VERSION(v) ((v >> 16) & 0xFF)
#define MINOR_VERSION(v) ((v >> 8) & 0xFF)
#define PATCH_VERSION(v) (v & 0xFF)

/* Public APIs */
extern "C"
{
    constexpr int MAX_ENVIRONMENT_VARIABLES = 256;

    /**
     * @brief The maximum memory for the sandboxed process, byte
     */
    constexpr uint64_t MAX_MEMORY_FOR_SANDBOX_PROCESS = 0x7FFFFFFF;

    enum SandboxSecurePolicy
    {
        DEFAULT     = 0,
        CXX_PROGRAM = 1,

        MAX_POLICY
    };

    struct SandboxConfiguration
    {
        const char *TaskName;               // The name of the sandboxed process, cannot be NULL
        const char *UserCommand;            // The command to run the sandboxed process, cannot be NULL
        const char *WorkingDirectory;       // The working directory for the sandboxed process
        const char *const *EnvironmentVariables; // The environment variables for the sandboxed process
        uint16_t EnvironmentVariablesCount; // The count of environment variables
        const char *InputFile;  // Redirect the input file for the sandboxed process, NULL means no redirection
        const char *OutputFile; // Redirect the output file for the sandboxed process, NULL means no redirection
        const char *ErrorFile;  // Redirect the error file for the sandboxed process, NULL means no redirection
        const char *LogFile;    // Redirect the log file for the sandboxed process, NULL means write to stderr

        /**
         * @brief
         * The maximum memory to crash the process, byte, 0 means two times of MaxMemory. (Hard limit)
         *
         * If the process exceeds MaxMemory limit but less than MaxMemoryToCrash, the process can still run,
         * but the memory usage will be recorded in the result (<code>SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED</code>).
         *
         * If the process exceeds MaxMemoryToCrash, the process will be terminated by kernel immediately.
         * result.Status will be <code>SANDBOX_RUNTIME_ERROR</code>, result.Signal will be SIGSEGV or SIGABRT.
         *
         * The value of MaxMemoryToCrash should be greater than MaxMemory.
         * If it is 0 or less than MaxMemory, it will use two times of MaxMemory.
         * @remarks
         * In Linux, setrlimit(RLIMIT_AS) to this value when the process exceeds this limit.
         */
        uint64_t MaxMemoryToCrash;

        uint64_t MaxMemory;     // The limit of memory, byte, 0 means no limit. (Soft limit)
        uint64_t MaxStack;      // The limit of stack, byte, 0 means no limit.
        uint64_t MaxCpuTime;    // The limit of CPU time, ms, 0 means no limit.
        uint64_t MaxRealTime;   // The limit of real time, ms, 0 means no limit.
        uint64_t MaxOutputSize; // The limit of output size, byte, 0 means no limit.
        int MaxProcessCount; // The limit of child process count, -1 means no limit.
        int Policy;          // The policy of the sandboxed process
    };

    struct SandboxResult
    {
        int Status;             // The status of the sandboxed process, see also the enum SandboxStatus
        int ExitCode;           // The exit code of the sandboxed process
        int Signal;             // The signal of the sandboxed process, if it is terminated by signal
        uint64_t CpuTimeUsage;  // The CPU time of the sandboxed process, ms
        uint64_t RealTimeUsage; // The real time of the sandboxed process, ms
        uint64_t MemoryUsage;   // The memory usage of the sandboxed process, byte
    };

    enum SandboxStatus
    {
        SANDBOX_STATUS_SUCCESS = 0,
        SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED,
        SANDBOX_STATUS_RUNTIME_ERROR,
        SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED,
        SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED,
        SANDBOX_STATUS_PROCESS_LIMIT_EXCEEDED,
        SANDBOX_STATUS_OUTPUT_LIMIT_EXCEEDED,
        SANDBOX_STATUS_ILLEGAL_OPERATION,

        SANDBOX_STATUS_INTERNAL_ERROR = 0xFFFF
    };

    /**
     * @brief Create and start a sandbox with the given configuration
     * @remark The function can block the current thread until the sandboxed process is terminated
     * @return SandboxStatus If the function succeeds, it returns SANDBOX_STATUS_SUCCESS
     */
    int StartSandbox(const SandboxConfiguration *config, SandboxResult *result);

    /**
     * @brief Check if the configuration is valid
     */
    bool IsSandboxConfigurationVaild(const SandboxConfiguration *config);
}

class SandboxImpl;

/**
 * @brief Sandbox
 * @remarks The constructor is private, use the static function Create to create a sandbox
 */
class Sandbox
{
    std::unique_ptr<SandboxImpl> _impl;

public:
    Sandbox() = default;

    struct CreateSandboxResult
    {
        int Status;
        std::unique_ptr<Sandbox> CreatedSandbox;
    };

    Sandbox(const Sandbox &)            = delete;
    Sandbox &operator=(const Sandbox &) = delete;
    Sandbox(Sandbox &&)                 = delete;
    Sandbox &operator=(Sandbox &&)      = delete;

    ~Sandbox() = default;

    /**
     * @brief Create a sandbox with the given configuration
     * @param config The configuration of the sandbox
     * @param result The result of the sandbox
     */
    static CreateSandboxResult Create(const SandboxConfiguration *config, SandboxResult &result);

    /**
     * @brief Get the version of the sandbox library
     * @return The version of the sandbox library
     */
    static constexpr int GetVersion()
    {
        return SANDBOX_VERSION;
    }

    /**
     * @brief Run the sandboxed process
     * @return The status of the sandboxed process
     */
    [[nodiscard]] int Run() const;
};

#endif // !SANDBOX_LIB_INC
