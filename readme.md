# Sandbox Runner (C++ implementation)

This project is part of the HimuOJ Project.
﻿
The SandboxRunnerCore library exposes a set of C APIs to:
﻿
- Establish an isolated sandbox process to run user-specified commands. 
- The process is immediately terminated upon attempting illegal system calls or exceeding resource limits, with the cause identified.
- Launch the sandbox using predefined configurations or specify detailed rules in parameters: the sandbox automatically translates these rules into seccomp rules at runtime.
﻿
The library currently supports only the Linux platform.

> The initial design goal of SandboxRunnerCore was an application sandbox library that could run on multiple platforms. 
However, on Windows and MacOS, due to system call limitations, only some features were supported or not supported at all. 
More heavyweight containers such as Docker should be used to implement on these systems.


﻿
SandboxRunner offers a straightforward command-line interface to utilize SandboxRunnerCore.


﻿
Typically, the SandboxRunnerCore library forms the core implementation of the code evaluation machines 
in code assessment platforms.