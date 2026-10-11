[CmdletBinding()]
param(
    [string]$QtRoot,
    [string]$BuildDirectory = "out/build/vs2022-x64"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

# Prefer an explicit Qt path, then QT_ROOT_DIR, then common C:\Qt installs.
$candidates = [System.Collections.Generic.List[string]]::new()
if (-not [string]::IsNullOrWhiteSpace($QtRoot)) {
    $candidates.Add($QtRoot)
}
if ([string]::IsNullOrWhiteSpace($QtRoot) -and -not [string]::IsNullOrWhiteSpace($env:QT_ROOT_DIR)) {
    $candidates.Add($env:QT_ROOT_DIR)
}
if ([string]::IsNullOrWhiteSpace($QtRoot) -and (Test-Path "C:\Qt")) {
    Get-ChildItem "C:\Qt" -Directory -ErrorAction SilentlyContinue |
        ForEach-Object {
            Get-ChildItem $_.FullName -Directory -Filter "msvc2022_64" -ErrorAction SilentlyContinue
        } |
        Sort-Object FullName -Descending |
        ForEach-Object { $candidates.Add($_.FullName) }
}

$resolvedQtRoot = $null
foreach ($candidate in ($candidates | Select-Object -Unique)) {
    if ([string]::IsNullOrWhiteSpace($candidate)) {
        continue
    }

    try {
        $resolvedCandidate = (Resolve-Path $candidate -ErrorAction Stop).Path
    }
    catch {
        continue
    }

    $qtConfig = Join-Path $resolvedCandidate "lib/cmake/Qt6/Qt6Config.cmake"
    if (Test-Path $qtConfig) {
        $resolvedQtRoot = $resolvedCandidate
        break
    }
}

if (-not $resolvedQtRoot) {
    throw @"
Qt 6 for MSVC 2022 x64 was not found.
Pass its installation directory explicitly, for example:
  .\scripts\configure-vs2022.ps1 -QtRoot "C:\Qt\6.8.3\msvc2022_64"
The directory must contain lib\cmake\Qt6\Qt6Config.cmake.
"@
}

$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmakeCommand) {
    throw "CMake 3.24+ was not found on PATH. Install CMake or enable the CMake component in Visual Studio 2022 and add its cmake.exe to PATH."
}

if ([System.IO.Path]::IsPathRooted($BuildDirectory)) {
    $resolvedBuildDirectory = $BuildDirectory
}
else {
    $resolvedBuildDirectory = Join-Path $repoRoot $BuildDirectory
}

Write-Host "Source:    $repoRoot"
Write-Host "Qt:        $resolvedQtRoot"
Write-Host "Build:     $resolvedBuildDirectory"
Write-Host "Generator: Visual Studio 17 2022 (x64)"

& $cmakeCommand.Source -S $repoRoot -B $resolvedBuildDirectory -G "Visual Studio 17 2022" -A x64 "-DCMAKE_PREFIX_PATH=$resolvedQtRoot" -DBUILD_TESTING=ON
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed with exit code $LASTEXITCODE."
}

$solutionPath = Join-Path $resolvedBuildDirectory "CLEOStudio.sln"
if (-not (Test-Path $solutionPath)) {
    throw "CMake reported success, but the expected Visual Studio solution was not found: $solutionPath"
}

Write-Host ""
Write-Host "Visual Studio 2022 solution created successfully:" -ForegroundColor Green
Write-Host "  $solutionPath"
Write-Host "Open it, select Release | x64, then build the solution."
Write-Host ('Run tests with: ctest --test-dir "{0}" -C Release --output-on-failure' -f $resolvedBuildDirectory)
