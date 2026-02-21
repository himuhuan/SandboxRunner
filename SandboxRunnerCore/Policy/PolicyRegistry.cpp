#include "PolicyRegistry.h"

#include "../Sandbox.h"

#include <cassert>
#include <seccomp.h>

namespace SandboxPolicyEngine
{
namespace
{

const SandboxPolicy kDefaultPolicy = {
    .PolicyId = DEFAULT,
    .Name = "DEFAULT",
    .AllowedSyscalls = {},
    .AllowedCapabilities = {},
    .PathAccessRules = {},
    .RestrictExecveToProgramPath = false,
    .AllowIO = true,
};

const SandboxPolicy kCxxProgramPolicy = {
    .PolicyId = CXX_PROGRAM,
    .Name = "CXX_PROGRAM",
    .AllowedSyscalls =
        {
            SCMP_SYS(access),
            SCMP_SYS(arch_prctl),
            SCMP_SYS(brk),
            SCMP_SYS(clock_gettime),
            SCMP_SYS(clock_nanosleep),
            SCMP_SYS(close),
            SCMP_SYS(exit_group),
            SCMP_SYS(faccessat),
            SCMP_SYS(fcntl),
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
            SCMP_SYS(pipe2),
            SCMP_SYS(read),
            SCMP_SYS(readlink),
            SCMP_SYS(readlinkat),
            SCMP_SYS(readv),
            SCMP_SYS(rseq),
            SCMP_SYS(set_robust_list),
            SCMP_SYS(set_tid_address),
            SCMP_SYS(write),
            SCMP_SYS(writev),
            SCMP_SYS(seccomp),
            SCMP_SYS(ioctl),
            SCMP_SYS(rt_sigprocmask),
        },
    .AllowedCapabilities = {},
    .PathAccessRules = {},
    .RestrictExecveToProgramPath = true,
    .AllowIO = true,
};

} // namespace

const SandboxPolicy *TryResolvePolicy(const int policyId)
{
    switch (policyId)
    {
    case DEFAULT:
        return &kDefaultPolicy;
    case CXX_PROGRAM:
        return &kCxxProgramPolicy;
    default:
        return nullptr;
    }
}

const SandboxPolicy &ResolvePolicy(const int policyId)
{
    const auto *policy = TryResolvePolicy(policyId);
    assert(policy != nullptr);
    return *policy;
}

bool IsKnownPolicy(const int policyId)
{
    return TryResolvePolicy(policyId) != nullptr;
}

} // namespace SandboxPolicyEngine
