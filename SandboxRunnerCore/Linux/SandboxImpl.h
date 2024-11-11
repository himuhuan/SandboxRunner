#pragma once
#include "../Sandbox.h"

constexpr int UNLIMITED = 0;

class SandboxImpl
{
private:
    const SandboxConfiguration *_config;
    SandboxResult &_result;

public:
    SandboxImpl(const SandboxConfiguration *config, SandboxResult &result);
    int Run();
};
