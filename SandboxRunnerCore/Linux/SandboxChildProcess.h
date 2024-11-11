#pragma once
#ifndef SANDBOX_CHILD_PROCESS_H
#define SANDBOX_CHILD_PROCESS_H

struct SandboxConfiguration;

void RunSandboxProcess(const char *programPath, char *const *programArgs, const SandboxConfiguration *configuration);

#endif //! SANDBOX_CHILD_PROCESS_H
