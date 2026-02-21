#include "SandboxTest.h"

#include "../SandboxRunnerCore/Policy/ResourceConfig.h"

namespace
{

SandboxConfiguration CreateValidConfiguration()
{
    SandboxConfiguration config{};
    config.TaskName    = "resource-config-test";
    config.UserCommand = "/bin/echo";
    config.Policy      = DEFAULT;

    config.MaxMemoryToCrash = 0;
    config.MaxMemory        = 128 * 1024 * 1024;
    config.MaxStack         = 8 * 1024 * 1024;
    config.MaxCpuTime       = 1000;
    config.MaxRealTime      = 3000;
    config.MaxOutputSize    = 64 * 1024;
    config.MaxProcessCount  = 1;

    return config;
}

} // namespace

TEST(ResourceConfigTest, ValidateNullConfiguration)
{
    const auto result = SandboxPolicyEngine::ValidateSandboxConfiguration(nullptr);
    EXPECT_FALSE(result.IsValid);
}

TEST(ResourceConfigTest, ValidateKnownConfiguration)
{
    auto config       = CreateValidConfiguration();
    const auto result = SandboxPolicyEngine::ValidateSandboxConfiguration(&config);
    EXPECT_TRUE(result.IsValid) << result.Message;
}

TEST(ResourceConfigTest, RejectUnknownPolicy)
{
    auto config  = CreateValidConfiguration();
    config.Policy = MAX_POLICY;

    const auto result = SandboxPolicyEngine::ValidateSandboxConfiguration(&config);
    EXPECT_FALSE(result.IsValid);
}

TEST(ResourceConfigTest, RejectTooLargeMemoryLimit)
{
    auto config     = CreateValidConfiguration();
    config.MaxMemory = MAX_MEMORY_FOR_SANDBOX_PROCESS + 1;

    const auto result = SandboxPolicyEngine::ValidateSandboxConfiguration(&config);
    EXPECT_FALSE(result.IsValid);
}

TEST(ResourceConfigTest, RejectInvalidProcessCount)
{
    auto config            = CreateValidConfiguration();
    config.MaxProcessCount = -2;

    const auto result = SandboxPolicyEngine::ValidateSandboxConfiguration(&config);
    EXPECT_FALSE(result.IsValid);
}

TEST(ResourceConfigTest, EffectiveMaxMemoryToCrashFallsBackToDoubleMemory)
{
    auto config = CreateValidConfiguration();
    config.MaxMemoryToCrash = 1;

    const auto resource = SandboxPolicyEngine::ResourceConfig::FromCConfig(config);
    EXPECT_EQ(resource.GetEffectiveMaxMemoryToCrash(), config.MaxMemory * 2);
}

TEST(ResourceConfigTest, EffectiveCpuLimitRoundsUpToSeconds)
{
    auto config = CreateValidConfiguration();
    config.MaxCpuTime = 1001;

    const auto resource = SandboxPolicyEngine::ResourceConfig::FromCConfig(config);
    EXPECT_EQ(resource.GetEffectiveCpuLimitSeconds(), 2);
}
