#pragma once

#include <cstdint>
#include <string>

struct SandboxConfiguration;

namespace SandboxPolicyEngine
{

struct ValidationResult
{
    bool IsValid = false;
    std::string Message;
};

struct ResourceConfig
{
    uint64_t MaxMemoryToCrash = 0;
    uint64_t MaxMemory = 0;
    uint64_t MaxStack = 0;
    uint64_t MaxCpuTime = 0;
    uint64_t MaxRealTime = 0;
    uint64_t MaxOutputSize = 0;
    int MaxProcessCount = 0;

    static ResourceConfig FromCConfig(const SandboxConfiguration &configuration);

    uint64_t GetEffectiveMaxMemoryToCrash() const;
    uint64_t GetEffectiveCpuLimitSeconds() const;
};

ValidationResult ValidateResourceConfig(const ResourceConfig &resourceConfig);
ValidationResult ValidateSandboxConfiguration(const SandboxConfiguration *configuration);

} // namespace SandboxPolicyEngine
