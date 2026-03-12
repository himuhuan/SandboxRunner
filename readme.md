# Sandbox Runner (C++ implementation)

This project is part of the HimuOJ Project.

The SandboxRunnerCore library exposes a set of C APIs to:

- Establish an isolated sandbox process to run user-specified commands.
- The process is immediately terminated upon attempting illegal system calls or exceeding resource limits, with the cause identified.
- Launch the sandbox using predefined configurations or specify detailed rules in parameters: the sandbox automatically translates these rules into seccomp rules at runtime.

The library currently supports only the Linux platform.

> The initial design goal of SandboxRunnerCore was an application sandbox library that could run on multiple platforms.
> However, on Windows and MacOS, due to system call limitations, only some features were supported or not supported at all.
> More heavyweight containers such as Docker should be used to implement on these systems.

SandboxRunner offers a straightforward command-line interface to utilize SandboxRunnerCore.

Typically, the SandboxRunnerCore library forms the core implementation of the code evaluation machines
in code assessment platforms.

---

## Building

The project uses CMake with vcpkg for dependency management.

```bash
# Debug build
cmake --preset linux-debug
cmake --build out/build/linux-debug --parallel

# Release build
cmake --preset linux-release
cmake --build out/build/linux-release --parallel
```

### Docker Release Validation

Day 0 release validation uses Docker on `ubuntu:22.04` to keep the build baseline aligned with the Linux deployment target while keeping the host environment flexible.

From `libs/SandboxRunner`, run:

```powershell
./scripts/export-jammy-release.ps1
```

This builds and tests `linux-release` inside Docker, then exports:

- `out/artifacts/linux-jammy-release/SandboxRunner`
- `out/artifacts/linux-jammy-release/libsandbox.so`
- `out/artifacts/linux-jammy-release/CXX_PROGRAM.json`

---

## CLI Usage

`SandboxRunner` is a command-line frontend for SandboxRunnerCore.

```
SandboxRunner [options] program [args...]
```

### Options

| Flag | Short | Description | Default |
|---|---|---|---|
| `--name` | `-n` | Task name (used in logs) | random UUID |
| `--dir` | `-d` | Working directory | `.` |
| `--input` | `-i` | Redirect stdin from file | (none) |
| `--output` | `-o` | Redirect stdout to file | (none) |
| `--error` | `-e` | Redirect stderr to file | (none) |
| `--log` | `-l` | Log file for sandbox diagnostics | stderr |
| `--memory` | | Memory soft limit, bytes (`0` = unlimited) | `0` |
| `--stack` | | Stack size limit, bytes (`0` = unlimited) | `0` |
| `--cpu` | | CPU time limit, ms (`0` = unlimited) | `0` |
| `--real` | | Real (wall-clock) time limit, ms (`0` = unlimited) | `0` |
| `--process` | | Max child process count (`-1` = unlimited) | `-1` |
| `--output-size` | | Output size limit, bytes (`0` = unlimited) | `0` |
| `--policy` | `-p` | Policy name or JSON file path | `default` |
| `--format` | `-f` | Result output format: `json` or `text` | `json` |

### Examples

Run a program with a 256 MB memory limit and 2-second time limit:

```bash
SandboxRunner --memory 268435456 --real 2000 ./solution
```

Run with I/O redirection and a custom policy:

```bash
SandboxRunner --input in.txt --output out.txt --policy cpp_policy ./solution
```

Run with strict process and output limits:

```bash
SandboxRunner --process 1 --output-size 65536 --cpu 1000 ./solution
```

Run and print a human-readable text result:

```bash
SandboxRunner --format text ./solution
```

---

## C API Usage

Include `Sandbox.h` and link against `SandboxRunnerCore`.

### Quick Start

```c
#include "Sandbox.h"

int main()
{
    SandboxConfiguration config = {};
    config.TaskName     = "my-task";
    config.UserCommand  = "/usr/bin/python3 solution.py";
    config.InputFile    = "input.txt";
    config.OutputFile   = "output.txt";
    config.MaxMemory    = 256 * 1024 * 1024; // 256 MB
    config.MaxRealTime  = 2000;              // 2 seconds
    config.MaxCpuTime   = 1000;              // 1 second CPU
    config.MaxProcessCount = 1;
    config.Policy       = "default";

    SandboxResult result = {};
    int status = StartSandbox(&config, &result);

    if (status == SANDBOX_STATUS_SUCCESS) {
        printf("Exit code: %d\n", result.ExitCode);
        printf("CPU time: %llu ms\n", result.CpuTimeUsage);
        printf("Memory:   %llu bytes\n", result.MemoryUsage);
    }
    return status;
}
```

### SandboxConfiguration Fields

| Field | Type | Description |
|---|---|---|
| `TaskName` | `const char *` | Task identifier used in logs. Must not be `NULL`. |
| `UserCommand` | `const char *` | Command to execute. Must not be `NULL`. |
| `WorkingDirectory` | `const char *` | Working directory for the child process. |
| `EnvironmentVariables` | `const char *const *` | Null-terminated array of `KEY=VALUE` strings. |
| `EnvironmentVariablesCount` | `uint16_t` | Number of entries in `EnvironmentVariables`. |
| `InputFile` | `const char *` | Redirect stdin. `NULL` = inherit. |
| `OutputFile` | `const char *` | Redirect stdout. `NULL` = inherit. |
| `ErrorFile` | `const char *` | Redirect stderr. `NULL` = inherit. |
| `LogFile` | `const char *` | Sandbox diagnostic log. `NULL` = write to stderr. |
| `MaxMemory` | `uint64_t` | Soft memory limit, bytes. `0` = no limit. |
| `MaxMemoryToCrash` | `uint64_t` | Hard memory limit, bytes. `0` = `2 × MaxMemory`. Exceeding this terminates the process with SIGSEGV/SIGABRT. |
| `MaxStack` | `uint64_t` | Stack size limit, bytes. `0` = no limit. |
| `MaxCpuTime` | `uint64_t` | CPU time limit, ms. `0` = no limit. |
| `MaxRealTime` | `uint64_t` | Wall-clock time limit, ms. `0` = no limit. |
| `MaxOutputSize` | `uint64_t` | Output size limit, bytes. `0` = no limit. |
| `MaxProcessCount` | `int` | Max child processes. `-1` = no limit. |
| `Policy` | `const char *` | Policy name or path (see [Policies](#policies)). `"default"` = unrestricted. |

**Memory limiting behavior:** If the process exceeds `MaxMemory` but stays below `MaxMemoryToCrash`, it continues running and the result status is set to `SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED`. If it exceeds `MaxMemoryToCrash`, the kernel terminates the process immediately and the result status is `SANDBOX_STATUS_RUNTIME_ERROR`.

### SandboxResult Fields

| Field | Type | Description |
|---|---|---|
| `Status` | `int` | Result code (see `SandboxStatus` enum) |
| `ExitCode` | `int` | Process exit code |
| `Signal` | `int` | Signal number if terminated by signal, otherwise `0` |
| `CpuTimeUsage` | `uint64_t` | CPU time consumed, ms |
| `RealTimeUsage` | `uint64_t` | Wall-clock time elapsed, ms |
| `MemoryUsage` | `uint64_t` | Peak memory usage, bytes |

### SandboxStatus Codes

| Value | Meaning |
|---|---|
| `SANDBOX_STATUS_SUCCESS` | Process exited normally |
| `SANDBOX_STATUS_MEMORY_LIMIT_EXCEEDED` | Memory usage exceeded `MaxMemory` |
| `SANDBOX_STATUS_RUNTIME_ERROR` | Process crashed (non-zero exit, signal, or hard memory limit) |
| `SANDBOX_STATUS_CPU_TIME_LIMIT_EXCEEDED` | CPU time exceeded `MaxCpuTime` |
| `SANDBOX_STATUS_REAL_TIME_LIMIT_EXCEEDED` | Wall-clock time exceeded `MaxRealTime` |
| `SANDBOX_STATUS_PROCESS_LIMIT_EXCEEDED` | Child process count exceeded `MaxProcessCount` |
| `SANDBOX_STATUS_OUTPUT_LIMIT_EXCEEDED` | Output size exceeded `MaxOutputSize` |
| `SANDBOX_STATUS_ILLEGAL_OPERATION` | Process attempted a syscall blocked by policy |
| `SANDBOX_STATUS_INTERNAL_ERROR` | Internal sandbox error |

### Validating Configuration

Use `IsSandboxConfigurationVaild()` to check a configuration before running:

```c
if (!IsSandboxConfigurationVaild(&config)) {
    fprintf(stderr, "Invalid sandbox configuration\n");
    return 1;
}
int status = StartSandbox(&config, &result);
```

---

## Policies

Policies control which syscalls the sandboxed process is allowed to make. The built-in `"default"` policy is unrestricted. Custom policies are defined in JSON files.

The production C++ execution policy is tracked at `policies/CXX_PROGRAM.json`. Downstream services should ship that file with the `SandboxRunner` binary and reference it by explicit path or working-directory-relative path at runtime.

### JSON Policy Format

Policy files use the `.json` extension. Pass the filename (with or without the extension) as the `Policy` field.

```json
{
    "Version": "1.0",
    "Seccomp": {
        "AllowIO": true,
        "RestrictExecveToProgramPath": false,
        "WhiteList": [
            "read",
            "write",
            "SCMP_SYS(exit_group)",
            "brk",
            60
        ]
    }
}
```

### Policy Fields

| Field | Type | Description |
|---|---|---|
| `Version` | string | Must be `"1.0"` |
| `Seccomp.AllowIO` | bool | Allow standard I/O syscalls (`read`, `write`, etc.). Default: `true` |
| `Seccomp.RestrictExecveToProgramPath` | bool | Restrict `execve` to the program path only. Default: `false` |
| `Seccomp.WhiteList` | array | Allowed syscalls as names, `SCMP_SYS(name)` macros, or integer numbers |

### Example: Minimal Policy for a C Program

```json
{
    "Version": "1.0",
    "Seccomp": {
        "AllowIO": true,
        "RestrictExecveToProgramPath": true,
        "WhiteList": [
            "read", "write", "exit_group",
            "brk", "mmap", "munmap",
            "fstat", "close"
        ]
    }
}
```

Save as `c_program.json` and reference it with `--policy c_program` (CLI) or `config.Policy = "c_program"` (API).
