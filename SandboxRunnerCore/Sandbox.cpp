#include "Linux/SandboxImpl.h"

Sandbox::CreateSandboxResult Sandbox::Create(const SandboxConfiguration *config, SandboxResult &result)
{
    auto sandbox = std::make_unique<Sandbox>();
    if (config == nullptr || config->TaskName == nullptr || config->UserCommand == nullptr)
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