#pragma once

#include <string>
#include <vector>

namespace SandboxPolicyEngine
{

enum class PathAccessMode
{
    ReadOnly,
    ReadWrite,
    NoAccess,
};

struct PathAccessRule
{
    std::string PathPrefix;
    PathAccessMode Mode = PathAccessMode::ReadOnly;
};

struct SandboxPolicy
{
    int PolicyId = 0;
    std::string Name;

    std::vector<int> AllowedSyscalls;
    std::vector<std::string> AllowedCapabilities;
    std::vector<PathAccessRule> PathAccessRules;

    bool RestrictExecveToProgramPath = false;
    bool AllowIO = false;
};

} // namespace SandboxPolicyEngine
