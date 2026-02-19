using System.Runtime.InteropServices;

internal static class Program
{
    private const int SandboxStatusSuccess = 0;
    private const int CxxProgramPolicy = 1;

    [StructLayout(LayoutKind.Sequential)]
    private struct SandboxConfiguration
    {
        public IntPtr TaskName;
        public IntPtr UserCommand;
        public IntPtr WorkingDirectory;
        public IntPtr EnvironmentVariables;
        public ushort EnvironmentVariablesCount;
        public IntPtr InputFile;
        public IntPtr OutputFile;
        public IntPtr ErrorFile;
        public IntPtr LogFile;
        public ulong MaxMemoryToCrash;
        public ulong MaxMemory;
        public ulong MaxStack;
        public ulong MaxCpuTime;
        public ulong MaxRealTime;
        public ulong MaxOutputSize;
        public int MaxProcessCount;
        public int Policy;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct SandboxResult
    {
        public int Status;
        public int ExitCode;
        public int Signal;
        public ulong CpuTimeUsage;
        public ulong RealTimeUsage;
        public ulong MemoryUsage;
    }

    [DllImport("sandbox", CallingConvention = CallingConvention.Cdecl, EntryPoint = "StartSandbox")]
    private static extern int StartSandbox(ref SandboxConfiguration configuration, out SandboxResult result);

    [DllImport("sandbox", CallingConvention = CallingConvention.Cdecl, EntryPoint = "IsSandboxConfigurationVaild")]
    [return: MarshalAs(UnmanagedType.I1)]
    private static extern bool IsSandboxConfigurationVaild(ref SandboxConfiguration configuration);

    private static int Main(string[] args)
    {
        var testsRoot = args.Length > 0 ? args[0] : Directory.GetCurrentDirectory();
        var samplePath = ResolveSamplePath(testsRoot, "ExpectedAccepted");
        var inputPath = Path.Combine(testsRoot, "TestData", "test_data.in");
        var outputPath = Path.Combine(testsRoot, "TestData", "pinvoke_smoke.out");

        if (!File.Exists(inputPath))
        {
            Console.Error.WriteLine($"Input file does not exist: {inputPath}");
            return 1;
        }

        if (File.Exists(outputPath))
        {
            File.Delete(outputPath);
        }

        var allocations = new List<IntPtr>();
        try
        {
            var configuration = new SandboxConfiguration
            {
                TaskName = ToNativeString("PInvokeSmoke", allocations),
                UserCommand = ToNativeString(samplePath, allocations),
                WorkingDirectory = ToNativeString(testsRoot, allocations),
                EnvironmentVariables = IntPtr.Zero,
                EnvironmentVariablesCount = 0,
                InputFile = ToNativeString(inputPath, allocations),
                OutputFile = ToNativeString(outputPath, allocations),
                ErrorFile = IntPtr.Zero,
                LogFile = IntPtr.Zero,
                MaxMemoryToCrash = 0,
                MaxMemory = 128UL * 1024 * 1024,
                MaxStack = 0,
                MaxCpuTime = 1000,
                MaxRealTime = 3000,
                MaxOutputSize = 10UL * 1024,
                MaxProcessCount = 0,
                Policy = CxxProgramPolicy,
            };

            if (!IsSandboxConfigurationVaild(ref configuration))
            {
                Console.Error.WriteLine("IsSandboxConfigurationVaild returned false");
                return 2;
            }

            var startStatus = StartSandbox(ref configuration, out var result);
            if (startStatus != SandboxStatusSuccess)
            {
                Console.Error.WriteLine($"StartSandbox returned {startStatus}");
                return 3;
            }

            if (result.Status != SandboxStatusSuccess || result.ExitCode != 0)
            {
                Console.Error.WriteLine($"Unexpected sandbox result: status={result.Status}, exit={result.ExitCode}");
                return 4;
            }

            Console.WriteLine("PInvoke smoke test passed");
            return 0;
        }
        finally
        {
            foreach (var pointer in allocations)
            {
                Marshal.FreeHGlobal(pointer);
            }
        }
    }

    private static string ResolveSamplePath(string testsRoot, string sampleName)
    {
        var noExt = Path.Combine(testsRoot, "Samples", sampleName);
        if (File.Exists(noExt))
        {
            return noExt;
        }

        var exe = noExt + ".exe";
        if (File.Exists(exe))
        {
            return exe;
        }

        throw new FileNotFoundException($"Sample executable not found: {noExt}");
    }

    private static IntPtr ToNativeString(string value, ICollection<IntPtr> allocations)
    {
        var pointer = Marshal.StringToHGlobalAnsi(value);
        allocations.Add(pointer);
        return pointer;
    }
}
