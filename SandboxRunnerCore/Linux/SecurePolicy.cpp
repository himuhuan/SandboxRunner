#include "SecurePolicy.h"

#include "../Logger.h"
#include "../InternalHelpers.h"
#include "../Policy/PolicyRegistry.h"

#include <cstdint>
#include <fcntl.h>
#include <seccomp.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace
{

bool AllowPolicySyscalls(scmp_filter_ctx ctx, const SandboxPolicyEngine::SandboxPolicy &policy)
{
    for (const auto syscall : policy.AllowedSyscalls)
    {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, syscall, 0) != 0)
        {
            return false;
        }
    }

    return true;
}

bool AllowExecveRule(scmp_filter_ctx ctx, const char *programPath, const SandboxPolicyEngine::SandboxPolicy &policy)
{
    if (policy.RestrictExecveToProgramPath)
    {
        // Keep legacy behavior for stage-1: preserve old seccomp rule shape.
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0,
                             SCMP_A0(SCMP_CMP_EQ, static_cast<scmp_datum_t>(reinterpret_cast<uintptr_t>(programPath)))))
        {
            return false;
        }
        return true;
    }

    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0) != 0)
    {
        return false;
    }

    return true;
}

bool AllowIoRules(scmp_filter_ctx ctx, const SandboxPolicyEngine::SandboxPolicy &policy)
{
    if (policy.AllowIO)
    {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0) != 0
            || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0) != 0
            || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup), 0) != 0
            || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup2), 0) != 0
            || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(dup3), 0) != 0)
        {
            return false;
        }
    }
    else
    {
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1,
                             SCMP_CMP(1, SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0)))
        {
            return false;
        }
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1,
                             SCMP_CMP(2, SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR, 0)))
        {
            return false;
        }
    }

    return true;
}

bool ApplyPolicy(const char *programPath, const SandboxPolicyEngine::SandboxPolicy &policy)
{
    SandboxInternal::SeccompContext ctx(SCMP_ACT_KILL);
    if (!ctx.valid())
    {
        return false;
    }

    if (!AllowPolicySyscalls(ctx.get(), policy))
    {
        return false;
    }

    if (!AllowExecveRule(ctx.get(), programPath, policy))
    {
        return false;
    }

    if (!AllowIoRules(ctx.get(), policy))
    {
        return false;
    }

    Logger::Info("Loading seccomp filter");
    if (seccomp_load(ctx.get()) != 0)
    {
        return false;
    }

    // SeccompContext destructor will automatically release the context
    return true;
}

} // namespace

bool ApplyLinuxSecurePolicy(const char *programPath, const SandboxConfiguration *config)
{
    const auto *policy = SandboxPolicyEngine::TryResolvePolicy(config->Policy);
    if (policy == nullptr)
    {
        return false;
    }

    if (policy->PolicyId == DEFAULT)
    {
        return true;
    }

    return ApplyPolicy(programPath, *policy);
}
