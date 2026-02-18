#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <pthread.h>

#ifdef __linux__
#include <seccomp.h>
#endif

// Forward declaration
struct SandboxConfiguration;

namespace SandboxInternal
{

// RAII wrapper for FILE*
struct FileDeleter
{
    void operator()(FILE *file) const
    {
        if (file != nullptr && file != stdin && file != stdout && file != stderr)
        {
            fclose(file);
        }
    }
};

using UniqueFile = std::unique_ptr<FILE, FileDeleter>;

#ifdef __linux__
// RAII wrapper for seccomp context
class SeccompContext
{
    scmp_filter_ctx _ctx;

public:
    explicit SeccompContext(uint32_t def_action) : _ctx(seccomp_init(def_action)) {}

    ~SeccompContext()
    {
        if (_ctx != nullptr)
        {
            seccomp_release(_ctx);
        }
    }

    SeccompContext(const SeccompContext &) = delete;
    SeccompContext &operator=(const SeccompContext &) = delete;
    SeccompContext(SeccompContext &&) = delete;
    SeccompContext &operator=(SeccompContext &&) = delete;

    scmp_filter_ctx get() const { return _ctx; }
    bool valid() const { return _ctx != nullptr; }
};
#endif

// RAII wrapper for pthread
class ScopedThread
{
    pthread_t _thread;
    bool _valid;

public:
    ScopedThread() : _thread(0), _valid(false) {}

    explicit ScopedThread(pthread_t thread) : _thread(thread), _valid(thread != 0) {}

    ~ScopedThread()
    {
        if (_valid && _thread != 0)
        {
            pthread_cancel(_thread);
            pthread_join(_thread, nullptr);
        }
    }

    ScopedThread(const ScopedThread &) = delete;
    ScopedThread &operator=(const ScopedThread &) = delete;
    ScopedThread(ScopedThread &&) = delete;
    ScopedThread &operator=(ScopedThread &&) = delete;

    pthread_t get() const { return _thread; }
    bool valid() const { return _valid; }

    void reset(pthread_t thread = 0)
    {
        if (_valid && _thread != 0)
        {
            pthread_cancel(_thread);
            pthread_join(_thread, nullptr);
        }
        _thread = thread;
        _valid = (thread != 0);
    }

    pthread_t release()
    {
        _valid = false;
        return _thread;
    }
};

// Internal configuration with modern C++ types
struct InternalConfig
{
    std::string TaskName;
    std::string UserCommand;
    std::string WorkingDirectory;
    std::vector<std::string> EnvironmentVariables;
    std::string InputFile;
    std::string OutputFile;
    std::string ErrorFile;
    std::string LogFile;

    uint64_t MaxMemoryToCrash;
    uint64_t MaxMemory;
    uint64_t MaxStack;
    uint64_t MaxCpuTime;
    uint64_t MaxRealTime;
    uint64_t MaxOutputSize;
    int MaxProcessCount;
    int Policy;

    // Convert from C ABI configuration
    static InternalConfig FromCConfig(const ::SandboxConfiguration *config);

    // Parse command into arguments
    std::vector<std::string> ParseCommandArgs() const;

    // Convert environment variables to char* const* for execve
    std::vector<char *> GetEnvPointers() const;
};

} // namespace SandboxInternal
