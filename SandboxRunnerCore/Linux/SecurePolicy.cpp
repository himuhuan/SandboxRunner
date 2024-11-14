#include "SecurePolicy.h"

#include "../Logger.h"

#include <cassert>
#include <seccomp.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace
{

// ReSharper disable once CppDFAConstantParameter
bool ApplyCxxProgramPolicy(const char *programPath, const SandboxConfiguration *config, bool allowIO)
{
    assert(config->Policy == CXX_PROGRAM);

    static constexpr int kAllowList[] = {
        SCMP_SYS(access),
        SCMP_SYS(arch_prctl),
        SCMP_SYS(brk),
        SCMP_SYS(clock_gettime),
        SCMP_SYS(close),
        SCMP_SYS(exit_group),
        SCMP_SYS(faccessat),
        SCMP_SYS(fstat),
        SCMP_SYS(futex),
        SCMP_SYS(flock),
        SCMP_SYS(getpid),
        SCMP_SYS(getrandom),
        SCMP_SYS(lseek),
        SCMP_SYS(mmap),
        SCMP_SYS(mprotect),
        SCMP_SYS(munmap),
        SCMP_SYS(newfstatat),
        SCMP_SYS(pread64),
        SCMP_SYS(prlimit64),
        SCMP_SYS(prctl),
        SCMP_SYS(read),
        SCMP_SYS(readlink),
        SCMP_SYS(readv),
        SCMP_SYS(rseq),
        SCMP_SYS(set_robust_list),
        SCMP_SYS(set_tid_address),
        SCMP_SYS(write),
        SCMP_SYS(writev),
        SCMP_SYS(seccomp),
        SCMP_SYS(set_tid_address),
        SCMP_SYS(rt_sigprocmask),

    // whitelist for debugging & testing
#if DEBUG
        SCMP_SYS(clock_nanosleep),
#endif
    };

    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    if (ctx == nullptr)
        return false;

    bool success = true;
    do
    {
        // Allow the list of syscalls
        for (const auto syscall : kAllowList)
        {
            if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, syscall, 0) != 0)
            {
                success = false;
                break;
            }
        }
        if (!success)
            break;

        // Only the program itself is allowed to be executed
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0,
                             SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)(programPath))))
        {
            success = false;
            break;
        }

        /* Always require IO? */
        // ReSharper disable once CppDFAConstantConditions
        if (allowIO)
        {
            if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0) != 0
                || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0) != 0
                || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup), 0) != 0
                || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup2), 0) != 0
                || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup3), 0) != 0)
            {
                success = false;
                break;
            }
        }
        else
        {
            // ReSharper disable once CppDFAUnreachableCode
            if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1,
                                 SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0)))
            {
                success = false;
                break;
            }
            if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1,
                                 SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0)))
            {
                success = false;
                break;
            }
        }

    } while (false);

    if (success)
    {
        Logger::Info("Loading seccomp filter");
        if (seccomp_load(ctx) != 0)
        {
            return false;
        }
    }

    seccomp_release(ctx);
    return success;
}

} // namespace

bool ApplyLinuxSecurePolicy(const char *programPath, const SandboxConfiguration *config)
{
    switch (config->Policy)
    {
    case CXX_PROGRAM:
        return ApplyCxxProgramPolicy(programPath, config, true);
    case DEFAULT:
        return true;
    default:
        [[unlikely]] return false;
    }
}
