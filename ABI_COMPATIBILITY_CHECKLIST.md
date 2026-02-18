# ABI Compatibility Checklist

This checklist is the frozen baseline for M1 and later stages. All items must stay green for every phase.

## C ABI Contract (must not break)

- `SandboxRunnerCore/Sandbox.h` keeps `extern "C"` boundary unchanged.
- `StartSandbox` and `IsSandboxConfigurationVaild` signatures stay unchanged.
- `SandboxConfiguration` field order, field types, and implicit alignment stay unchanged.
- `SandboxResult` field order, field types, and implicit alignment stay unchanged.
- Existing `SandboxStatus` enum numeric semantics stay unchanged.
- Existing `SandboxSecurePolicy` enum numeric semantics stay unchanged.

## Automated Guards

- `Tests/AbiCompatibilityTest.cpp` static asserts:
  - API function pointer signatures
  - enum numeric values
  - `sizeof` and `offsetof` against frozen baseline structs
- `Tests/AbiCompatibilityTest.cpp` runtime exported symbol checks:
  - `StartSandbox`
  - `IsSandboxConfigurationVaild`
- `Tests/PInvokeSmoke/Program.cs` .NET P/Invoke smoke checks:
  - marshaling `SandboxConfiguration`
  - calling `IsSandboxConfigurationVaild`
  - calling `StartSandbox`
  - reading `SandboxResult`

## M1 Acceptance

- Build scripts modernized to target-based CMake usage.
- Warning baselines enabled (non-fatal) for MSVC and non-MSVC.
- Existing artifacts still produced (`sandbox` shared library, `SandboxRunner`, `SandboxTest`).
- `AllTestsInMain` and `PInvokeSmoke` pass in supported environments.
