#include "PolicyRegistry.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <seccomp.h>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace SandboxPolicyEngine
{
namespace
{

using Json = nlohmann::json;

const SandboxPolicy kDefaultPolicy = {
    .Name = std::string(DEFAULT_POLICY_NAME),
    .AllowedSyscalls = {},
    .AllowedCapabilities = {},
    .PathAccessRules = {},
    .RestrictExecveToProgramPath = false,
    .AllowIO = true,
};

std::mutex gPolicyCacheMutex;
std::unordered_map<std::string, std::shared_ptr<SandboxPolicy>> gPolicyCache;

std::string TrimPolicyToken(std::string_view token)
{
    size_t begin = 0;
    while (begin < token.size() && std::isspace(static_cast<unsigned char>(token[begin])))
    {
        ++begin;
    }

    size_t end = token.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(token[end - 1])))
    {
        --end;
    }

    return std::string(token.substr(begin, end - begin));
}

std::string NormalizePolicyName(std::string_view policyName)
{
    auto normalized = TrimPolicyToken(policyName);
    if (normalized.empty())
    {
        normalized = std::string(DEFAULT_POLICY_NAME);
    }

    return normalized;
}

std::filesystem::path BuildPolicyPath(const std::string &policyName)
{
    std::filesystem::path path(policyName);
    if (path.extension() != ".json")
    {
        path += ".json";
    }
    return path;
}

std::optional<int> TryResolveSyscall(const Json &syscallNode)
{
    if (syscallNode.is_number_integer())
    {
        return syscallNode.get<int>();
    }

    if (!syscallNode.is_string())
    {
        return std::nullopt;
    }

    std::string syscallName = TrimPolicyToken(syscallNode.get<std::string>());
    if (syscallName.empty())
    {
        return std::nullopt;
    }

    constexpr std::string_view kScmpMacroPrefix = "SCMP_SYS(";
    if (syscallName.rfind(kScmpMacroPrefix, 0) == 0 && syscallName.back() == ')')
    {
        syscallName = syscallName.substr(kScmpMacroPrefix.size(),
                                         syscallName.size() - kScmpMacroPrefix.size() - 1);
    }

    const int syscallId = seccomp_syscall_resolve_name(syscallName.c_str());
    if (syscallId == __NR_SCMP_ERROR)
    {
        return std::nullopt;
    }

    return syscallId;
}

std::optional<SandboxPolicy> LoadPolicyFromFile(const std::string &policyName)
{
    std::ifstream input(BuildPolicyPath(policyName));
    if (!input.is_open())
    {
        return std::nullopt;
    }

    Json root;
    try
    {
        input >> root;
    }
    catch (const Json::exception &)
    {
        return std::nullopt;
    }

    if (!root.is_object())
    {
        return std::nullopt;
    }

    const auto versionIt = root.find("Version");
    if (versionIt == root.end() || !versionIt->is_string() || versionIt->get<std::string>() != "1.0")
    {
        return std::nullopt;
    }

    const auto seccompIt = root.find("Seccomp");
    if (seccompIt == root.end() || !seccompIt->is_object())
    {
        return std::nullopt;
    }

    const auto whiteListIt = seccompIt->find("WhiteList");
    if (whiteListIt == seccompIt->end() || !whiteListIt->is_array())
    {
        return std::nullopt;
    }

    SandboxPolicy policy;
    policy.Name = policyName;

    const auto allowIoIt = seccompIt->find("AllowIO");
    if (allowIoIt != seccompIt->end())
    {
        if (!allowIoIt->is_boolean())
        {
            return std::nullopt;
        }
        policy.AllowIO = allowIoIt->get<bool>();
    }
    else
    {
        policy.AllowIO = true;
    }

    const auto restrictExecveIt = seccompIt->find("RestrictExecveToProgramPath");
    if (restrictExecveIt != seccompIt->end())
    {
        if (!restrictExecveIt->is_boolean())
        {
            return std::nullopt;
        }
        policy.RestrictExecveToProgramPath = restrictExecveIt->get<bool>();
    }

    for (const auto &syscallNode : *whiteListIt)
    {
        const auto syscall = TryResolveSyscall(syscallNode);
        if (!syscall.has_value())
        {
            return std::nullopt;
        }
        policy.AllowedSyscalls.push_back(*syscall);
    }

    std::sort(policy.AllowedSyscalls.begin(), policy.AllowedSyscalls.end());
    policy.AllowedSyscalls.erase(std::unique(policy.AllowedSyscalls.begin(), policy.AllowedSyscalls.end()),
                                 policy.AllowedSyscalls.end());

    return policy;
}

} // namespace

bool IsDefaultPolicyName(const std::string_view policyName)
{
    return NormalizePolicyName(policyName) == DEFAULT_POLICY_NAME;
}

bool IsDefaultPolicyName(const char *policyName)
{
    if (policyName == nullptr)
    {
        return true;
    }

    return IsDefaultPolicyName(std::string_view(policyName));
}

const SandboxPolicy *TryResolvePolicy(const std::string_view policyName)
{
    const auto normalizedPolicyName = NormalizePolicyName(policyName);
    if (normalizedPolicyName == DEFAULT_POLICY_NAME)
    {
        return &kDefaultPolicy;
    }

    std::lock_guard lock(gPolicyCacheMutex);
    if (const auto it = gPolicyCache.find(normalizedPolicyName); it != gPolicyCache.end())
    {
        return it->second.get();
    }

    auto loadedPolicy = LoadPolicyFromFile(normalizedPolicyName);
    if (!loadedPolicy.has_value())
    {
        return nullptr;
    }

    auto policy = std::make_shared<SandboxPolicy>(std::move(*loadedPolicy));
    const auto *resolvedPolicy = policy.get();
    gPolicyCache.emplace(normalizedPolicyName, std::move(policy));
    return resolvedPolicy;
}

const SandboxPolicy *TryResolvePolicy(const char *policyName)
{
    if (policyName == nullptr)
    {
        return &kDefaultPolicy;
    }

    return TryResolvePolicy(std::string_view(policyName));
}

const SandboxPolicy *TryResolvePolicyNoCache(const std::string_view policyName, SandboxPolicy &storage)
{
    const auto normalizedPolicyName = NormalizePolicyName(policyName);
    if (normalizedPolicyName == DEFAULT_POLICY_NAME)
    {
        return &kDefaultPolicy;
    }

    auto loadedPolicy = LoadPolicyFromFile(normalizedPolicyName);
    if (!loadedPolicy.has_value())
    {
        return nullptr;
    }

    storage = std::move(*loadedPolicy);
    return &storage;
}

const SandboxPolicy *TryResolvePolicyNoCache(const char *policyName, SandboxPolicy &storage)
{
    if (policyName == nullptr)
    {
        return &kDefaultPolicy;
    }

    return TryResolvePolicyNoCache(std::string_view(policyName), storage);
}

const SandboxPolicy &ResolvePolicy(const std::string_view policyName)
{
    const auto *policy = TryResolvePolicy(policyName);
    assert(policy != nullptr);
    return *policy;
}

bool IsKnownPolicy(const std::string_view policyName)
{
    return TryResolvePolicy(policyName) != nullptr;
}

bool IsKnownPolicy(const char *policyName)
{
    return TryResolvePolicy(policyName) != nullptr;
}

} // namespace SandboxPolicyEngine
