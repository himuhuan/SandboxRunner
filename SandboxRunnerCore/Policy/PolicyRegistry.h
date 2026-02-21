#pragma once

#include "SandboxPolicy.h"

namespace SandboxPolicyEngine
{

const SandboxPolicy *TryResolvePolicy(int policyId);
const SandboxPolicy &ResolvePolicy(int policyId);
bool IsKnownPolicy(int policyId);

} // namespace SandboxPolicyEngine
