#include "SandboxTest.h"

#include "../SandboxRunnerCore/Policy/PolicyRegistry.h"

#include <algorithm>
#include <seccomp.h>

namespace
{

bool ContainsSyscall(const std::vector<int> &syscalls, int syscall)
{
    return std::find(syscalls.begin(), syscalls.end(), syscall) != syscalls.end();
}

} // namespace

TEST(PolicyRegistryTest, ResolveDefaultPolicy)
{
    const auto *policy = SandboxPolicyEngine::TryResolvePolicy(DEFAULT);
    ASSERT_NE(policy, nullptr);

    EXPECT_EQ(policy->PolicyId, DEFAULT);
    EXPECT_EQ(policy->Name, "DEFAULT");
    EXPECT_TRUE(policy->AllowedSyscalls.empty());
}

TEST(PolicyRegistryTest, ResolveCxxProgramPolicy)
{
    const auto *policy = SandboxPolicyEngine::TryResolvePolicy(CXX_PROGRAM);
    ASSERT_NE(policy, nullptr);

    EXPECT_EQ(policy->PolicyId, CXX_PROGRAM);
    EXPECT_EQ(policy->Name, "CXX_PROGRAM");
    EXPECT_TRUE(policy->RestrictExecveToProgramPath);
    EXPECT_TRUE(policy->AllowIO);
    EXPECT_FALSE(policy->AllowedSyscalls.empty());
}

TEST(PolicyRegistryTest, CxxProgramPolicyContainsCriticalSyscalls)
{
    const auto *policy = SandboxPolicyEngine::TryResolvePolicy(CXX_PROGRAM);
    ASSERT_NE(policy, nullptr);

    EXPECT_TRUE(ContainsSyscall(policy->AllowedSyscalls, SCMP_SYS(read)));
    EXPECT_TRUE(ContainsSyscall(policy->AllowedSyscalls, SCMP_SYS(write)));
    EXPECT_TRUE(ContainsSyscall(policy->AllowedSyscalls, SCMP_SYS(seccomp)));
    EXPECT_TRUE(ContainsSyscall(policy->AllowedSyscalls, SCMP_SYS(prlimit64)));
    EXPECT_TRUE(ContainsSyscall(policy->AllowedSyscalls, SCMP_SYS(fcntl)));
}

TEST(PolicyRegistryTest, CxxProgramPolicyMatchesLegacyAllowList)
{
    const auto *policy = SandboxPolicyEngine::TryResolvePolicy(CXX_PROGRAM);
    ASSERT_NE(policy, nullptr);

    const std::vector<int> expected = {
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
    };

    EXPECT_EQ(policy->AllowedSyscalls.size(), expected.size());
    for (const auto syscall : expected)
    {
        EXPECT_TRUE(ContainsSyscall(policy->AllowedSyscalls, syscall));
    }
}

TEST(PolicyRegistryTest, UnknownPolicyReturnsNull)
{
    EXPECT_EQ(SandboxPolicyEngine::TryResolvePolicy(MAX_POLICY), nullptr);
    EXPECT_FALSE(SandboxPolicyEngine::IsKnownPolicy(MAX_POLICY));
}
