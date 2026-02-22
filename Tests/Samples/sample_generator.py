# -*- coding: utf-8 -*-

# File: sample_bootstrap.py
# Usage: sample_bootstrap.py OutputDir CompilerName
# Compile all C++source files in the current directory using the specified compiler
# and OutputDir as the output directory for the object files.
# this script is used in CMakeLists.txt to generate the executables for the tests.

import os
import sys
import subprocess

if len(sys.argv) != 3:
    print("Usage: sample_bootstrap.py OutputDir CompilerName")
    sys.exit(1)

output_dir = sys.argv[1]
compiler_name = sys.argv[2]

print(f"Starting compilation of C++ files using {compiler_name} to {output_dir}...")

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

cpp_files = [f for f in os.listdir('.') if f.endswith('.cpp')]

for cpp_file in cpp_files:
    output_file = os.path.join(output_dir, os.path.splitext(cpp_file)[0])
    command = [compiler_name, '-std=c++20', '-O2', cpp_file, '-o', output_file]
    try:
        subprocess.run(command, check=True)
        print(f"Compiled {cpp_file} to {output_file}")
    except subprocess.CalledProcessError as e:
        print(f"Error compiling {cpp_file}: {e}")
        sys.exit(1)

print("All C++ files compiled successfully.")
