#pragma once

#include "../Sandbox.h"
#include <cerrno>
#include <cstring>
#include <string_view>

namespace SandboxInternal
{

/**
 * @brief Internal error categories with finer granularity than public SandboxStatus
 */
enum class InternalError
{
    // Configuration errors
    InvalidWorkingDirectory,
    InvalidCommandArgs,

    // Resource limit errors
    ResourceLimitFailed,

    // File I/O errors
    InputFileOpenFailed,
    OutputFileOpenFailed,
    ErrorFileOpenFailed,
    FileRedirectFailed,

    // Process management errors
    ForkFailed,
    ExecFailed,
    WaitFailed,

    // Security policy errors
    PolicyApplicationFailed,

    // Monitor thread errors
    MonitorThreadStartFailed,

    // Generic internal error
    Unknown
};

/**
 * @brief Error context containing all information about an error
 */
struct ErrorContext
{
    InternalError error;
    int savedErrno;
    std::string_view message;

    ErrorContext(InternalError err, std::string_view msg)
        : error(err), savedErrno(errno), message(msg)
    {
    }
};

/**
 * @brief Maps internal error to public SandboxStatus
 */
inline int MapErrorToStatus(InternalError error)
{
    // All internal errors map to SANDBOX_STATUS_INTERNAL_ERROR
    // This provides a single point for error classification
    return SANDBOX_STATUS_INTERNAL_ERROR;
}

/**
 * @brief Handles error in parent process context
 * @return The appropriate SandboxStatus code
 */
int HandleParentError(const ErrorContext &ctx);

/**
 * @brief Handles error in child process context (does not return)
 */
[[noreturn]] void HandleChildError(const ErrorContext &ctx);

} // namespace SandboxInternal
