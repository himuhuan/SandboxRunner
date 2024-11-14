#ifndef SANDBOX_SECURE_POLICY_H
#define SANDBOX_SECURE_POLICY_H
#include "../Sandbox.h"

bool ApplyLinuxSecurePolicy(const char *programPath, const SandboxConfiguration *config);

#endif // SANDBOX_SECURE_POLICY_H
