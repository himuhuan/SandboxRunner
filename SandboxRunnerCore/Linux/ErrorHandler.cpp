#include "ErrorHandler.h"
#include "../Logger.h"
#include <csignal>
#include <unistd.h>

namespace SandboxInternal
{

int HandleParentError(const ErrorContext &ctx)
{
    const char *errorType = [](InternalError err) -> const char * {
        switch (err)
        {
        case InternalError::InvalidWorkingDirectory:
            return "Invalid working directory";
        case InternalError::InvalidCommandArgs:
            return "Invalid command arguments";
        case InternalError::ResourceLimitFailed:
            return "Resource limit setup failed";
        case InternalError::InputFileOpenFailed:
            return "Input file open failed";
        case InternalError::OutputFileOpenFailed:
            return "Output file open failed";
        case InternalError::ErrorFileOpenFailed:
            return "Error file open failed";
        case InternalError::FileRedirectFailed:
            return "File redirect failed";
        case InternalError::ForkFailed:
            return "Fork failed";
        case InternalError::ExecFailed:
            return "Exec failed";
        case InternalError::WaitFailed:
            return "Wait failed";
        case InternalError::PolicyApplicationFailed:
            return "Policy application failed";
        case InternalError::MonitorThreadStartFailed:
            return "Monitor thread start failed";
        default:
            return "Unknown error";
        }
    }(ctx.error);

    if (ctx.savedErrno != 0)
    {
        Logger::Error("{0}: {1}, errno: {2} ({3})",
                     errorType, ctx.message, ctx.savedErrno, strerror(ctx.savedErrno));
    }
    else
    {
        Logger::Error("{0}: {1}", errorType, ctx.message);
    }

    return MapErrorToStatus(ctx.error);
}

[[noreturn]] void HandleChildError(const ErrorContext &ctx)
{
    const char *errorType = [](InternalError err) -> const char * {
        switch (err)
        {
        case InternalError::InvalidWorkingDirectory:
            return "Invalid working directory";
        case InternalError::ResourceLimitFailed:
            return "Resource limit setup failed";
        case InternalError::InputFileOpenFailed:
            return "Input file open failed";
        case InternalError::OutputFileOpenFailed:
            return "Output file open failed";
        case InternalError::ErrorFileOpenFailed:
            return "Error file open failed";
        case InternalError::FileRedirectFailed:
            return "File redirect failed";
        case InternalError::ExecFailed:
            return "Exec failed";
        case InternalError::PolicyApplicationFailed:
            return "Policy application failed";
        default:
            return "Unknown error";
        }
    }(ctx.error);

    if (ctx.savedErrno != 0)
    {
        Logger::Error("(CHILD {0}): {1}: {2}, errno: {3} ({4})",
                     static_cast<int>(getpid()), errorType, ctx.message,
                     ctx.savedErrno, strerror(ctx.savedErrno));
    }
    else
    {
        Logger::Error("(CHILD {0}): {1}: {2}",
                     static_cast<int>(getpid()), errorType, ctx.message);
    }

    // Signal parent that child encountered fatal error
    raise(SIGUSR1);
    exit(MapErrorToStatus(ctx.error));
}

} // namespace SandboxInternal