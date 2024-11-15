#include "Linux/SandboxImpl.h"

Sandbox::CreateSandboxResult Sandbox::Create(const SandboxConfiguration *config, SandboxResult &result)
{
    auto sandbox = std::make_unique<Sandbox>();
    if (IsSandboxConfigurationVaild(config) == false)
        return {SANDBOX_STATUS_INTERNAL_ERROR, nullptr};
    sandbox->_impl = std::make_unique<SandboxImpl>(config, result);
    if (sandbox->_impl == nullptr)
        return {SANDBOX_STATUS_INTERNAL_ERROR, nullptr};
    return {SANDBOX_STATUS_SUCCESS, std::move(sandbox)};
}

int Sandbox::Run() const
{
    return _impl->Run();
}

int StartSandbox(const SandboxConfiguration *config, SandboxResult *result)
{
    auto [status, sandbox] = Sandbox::Create(config, *result);
    if (status != SANDBOX_STATUS_SUCCESS)
        return status;
    return sandbox->Run();
}

bool IsSandboxConfigurationVaild(const SandboxConfiguration *config)
{
    if (config == nullptr || config->TaskName == nullptr || config->UserCommand == nullptr)
        return false;
    if (config->Policy < 0 || config->Policy >= MAX_POLICY)
        return false;
    if (config->MaxMemory > MAX_MEMORY_FOR_SANDBOX_PROCESS)
        return false;
    if (config->MaxMemoryToCrash != 0 && config->MaxMemoryToCrash < (config->MaxMemory << 1))
        return false;
    return true;
}
