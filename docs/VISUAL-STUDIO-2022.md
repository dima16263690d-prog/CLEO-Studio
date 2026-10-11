# Visual Studio 2022 (Windows x64)

CLEO Studio uses CMake for its target graph. The supported Visual Studio workflow generates a standard CLEOStudio.sln and .vcxproj files in the local build directory, rather than keeping generated machine-specific project files in source control. The solution includes:

- cleo-studio — Qt GUI application
- cleo-studio-cli — command-line utility
- cleo_core — reusable C++20 library
- cleo-core-tests — regression tests

## Prerequisites

1. Visual Studio 2022 with **Desktop development with C++**, MSVC x64 tools, a Windows SDK, and CMake support. The repository-root .vsconfig lists the requested VS components.
2. CMake 3.24 or newer.
3. Qt 6.5 or newer built for **MSVC 2022 64-bit**, with Qt Widgets. Do not mix MinGW Qt with the MSVC compiler.

## One-command solution generation

Open PowerShell in the repository root and run:

    .\scripts\configure-vs2022.ps1 -QtRoot "C:\Qt\6.8.3\msvc2022_64"

Change the path to the directory containing Qt's lib\cmake\Qt6\Qt6Config.cmake. If your Qt installation is under the standard C:\Qt\<version>\msvc2022_64 layout, the script can attempt to detect it automatically:

    .\scripts\configure-vs2022.ps1

After successful configuration, open:

    out\build\vs2022-x64\CLEOStudio.sln

In Visual Studio, choose **Release | x64**, then **Build → Build Solution**. Set cleo-studio as the startup project when you want to run the GUI. CLI and test projects are part of the same solution.

## Build and test from PowerShell

    cmake --build out/build/vs2022-x64 --config Release --parallel
    ctest --test-dir out/build/vs2022-x64 -C Release --output-on-failure

## Manual equivalent

The setup script runs the equivalent CMake command:

    cmake -S . -B out/build/vs2022-x64 -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2022_64" -DBUILD_TESTING=ON

The generated solution is machine-local build output and is intentionally ignored by Git. Source lists, compiler settings and project dependencies remain centralized in CMakeLists.txt, so the Visual Studio build and CI build use the same definitions.

## Troubleshooting

- **Qt6Config.cmake not found:** pass the Qt installation root, not its bin directory. It must contain lib/cmake/Qt6/Qt6Config.cmake.
- **Wrong compiler/Qt kit:** install the Qt kit labelled msvc2022_64; MinGW kits are not compatible with MSVC-built targets.
- **CMake not found:** install CMake 3.24+ and ensure cmake.exe is on PATH.
- **Solution missing after configuration:** read the CMake error above; the script checks that CLEOStudio.sln was actually generated before reporting success.
