#pragma once

#include "SandboxPolicy.h"

#include <string_view>

namespace SandboxPolicyEngine
{

inline constexpr std::string_view DEFAULT_POLICY_NAME = "default";

bool IsDefaultPolicyName(std::string_view policyName);
bool IsDefaultPolicyName(const char *policyName);
const SandboxPolicy *TryResolvePolicy(std::string_view policyName);
const SandboxPolicy *TryResolvePolicy(const char *policyName);
const SandboxPolicy &ResolvePolicy(std::string_view policyName);
bool IsKnownPolicy(std::string_view policyName);
bool IsKnownPolicy(const char *policyName);

} // namespace SandboxPolicyEngine
