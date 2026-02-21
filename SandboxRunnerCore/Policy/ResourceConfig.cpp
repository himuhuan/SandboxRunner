#include "ResourceConfig.h"

#include "PolicyRegistry.h"
#include "../Sandbox.h"

#include <utility>

namespace SandboxPolicyEngine
{
namespace
{

ValidationResult CreateInvalidResult(std::string message)
{
    return {.IsValid = false, .Message = std::move(message)};
}

ValidationResult CreateValidResult()
{
    return {.IsValid = true, .Message = {}};
}

} // namespace

ResourceConfig ResourceConfig::FromCConfig(const SandboxConfiguration &configuration)
{
    return {
        .MaxMemoryToCrash = configuration.MaxMemoryToCrash,
        .MaxMemory = configuration.MaxMemory,
        .MaxStack = configuration.MaxStack,
        .MaxCpuTime = configuration.MaxCpuTime,
        .MaxRealTime = configuration.MaxRealTime,
        .MaxOutputSize = configuration.MaxOutputSize,
        .MaxProcessCount = configuration.MaxProcessCount,
    };
}

uint64_t ResourceConfig::GetEffectiveMaxMemoryToCrash() const
{
    if (MaxMemoryToCrash != 0 && (MaxMemory == 0 || MaxMemoryToCrash >= MaxMemory))
    {
        return MaxMemoryToCrash;
    }

    if (MaxMemory == 0)
    {
        return 0;
    }

    return MaxMemory * 2;
}

uint64_t ResourceConfig::GetEffectiveCpuLimitSeconds() const
{
    if (MaxCpuTime == 0)
    {
        return 0;
    }

    return (MaxCpuTime + 1000) / 1000;
}

ValidationResult ValidateResourceConfig(const ResourceConfig &resourceConfig)
{
    if (resourceConfig.MaxMemory > MAX_MEMORY_FOR_SANDBOX_PROCESS)
    {
        return CreateInvalidResult("MaxMemory exceeds MAX_MEMORY_FOR_SANDBOX_PROCESS");
    }

    if (resourceConfig.MaxProcessCount < -1)
    {
        return CreateInvalidResult("MaxProcessCount cannot be less than -1");
    }

    return CreateValidResult();
}

ValidationResult ValidateSandboxConfiguration(const SandboxConfiguration *configuration)
{
    if (configuration == nullptr)
    {
        return CreateInvalidResult("configuration cannot be null");
    }

    if (configuration->TaskName == nullptr || configuration->UserCommand == nullptr)
    {
        return CreateInvalidResult("TaskName and UserCommand must be set");
    }

    if (!IsKnownPolicy(configuration->Policy))
    {
        return CreateInvalidResult("Policy value is out of range");
    }

    if (configuration->EnvironmentVariablesCount > MAX_ENVIRONMENT_VARIABLES)
    {
        return CreateInvalidResult("EnvironmentVariablesCount exceeds MAX_ENVIRONMENT_VARIABLES");
    }

    if (configuration->EnvironmentVariablesCount != 0 && configuration->EnvironmentVariables == nullptr)
    {
        return CreateInvalidResult("EnvironmentVariables is required when EnvironmentVariablesCount > 0");
    }

    return ValidateResourceConfig(ResourceConfig::FromCConfig(*configuration));
}

} // namespace SandboxPolicyEngine
