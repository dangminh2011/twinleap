[CmdletBinding()]
param(
    [string]$SDL3Dir = '',
    [switch]$Run,
    [switch]$Package
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Invoke-Checked {
    param([string]$Program, [string[]]$Arguments)
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed (exit $LASTEXITCODE)." }
}

Push-Location -LiteralPath $PSScriptRoot
try {
    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCommand) { throw 'Install CMake >= 3.21 and add it to PATH.' }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw 'Install Visual Studio or Build Tools with Desktop development with C++.'
    }
    $vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) { throw 'Visual Studio C++ x64 tools were not found.' }
    $vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
    # Import the developer environment only into this PowerShell process.
    $developerEnvironment = & $env:ComSpec /d /c "`"$vcvars`" >nul && set"
    if ($LASTEXITCODE -ne 0) { throw 'Could not initialize the MSVC x64 environment.' }
    foreach ($entry in $developerEnvironment) {
        $separator = $entry.IndexOf('=')
        if ($separator -gt 0) {
            [Environment]::SetEnvironmentVariable($entry.Substring(0, $separator), $entry.Substring($separator + 1), 'Process')
        }
    }
    $ninjaCommand = Get-Command ninja -ErrorAction SilentlyContinue
    $ninjaPath = if ($ninjaCommand) { $ninjaCommand.Source } else {
        Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe'
    }
    if (-not (Test-Path -LiteralPath $ninjaPath)) { throw 'Install Ninja or the Visual Studio CMake tools component.' }

    if (-not $SDL3Dir -and -not $env:SDL3_DIR -and (Test-Path -LiteralPath '.deps')) {
        $localSDL = Get-ChildItem -LiteralPath '.deps' -Directory -Filter 'SDL3-*' |
            Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'cmake\SDL3Config.cmake') } |
            Sort-Object { [version]($_.Name.Substring(5)) } -Descending | Select-Object -First 1
        if ($localSDL) { $SDL3Dir = Join-Path $localSDL.FullName 'cmake' }
    }
    $configure = @('-S', '.', '-B', 'build-windows', '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=Release', "-DCMAKE_MAKE_PROGRAM=$ninjaPath", '-DTWINLEAP_BUILD_TESTS=ON')
    if ($SDL3Dir) { $configure += "-DSDL3_DIR=$SDL3Dir" }
    elseif ($env:SDL3_DIR) { $configure += "-DSDL3_DIR=$env:SDL3_DIR" }
    Invoke-Checked $cmakeCommand.Source $configure
    Invoke-Checked $cmakeCommand.Source @('--build', 'build-windows', '--parallel')
    $ctest = Join-Path (Split-Path $cmakeCommand.Source) 'ctest.exe'
    Invoke-Checked $ctest @('--test-dir', 'build-windows', '--output-on-failure')
    if ($Package) {
        $cpack = Join-Path (Split-Path $cmakeCommand.Source) 'cpack.exe'
        Invoke-Checked $cpack @('--config', 'build-windows/CPackConfig.cmake', '-B', 'build-windows/package')
        Write-Host 'Package: build-windows/package/Twinleap-Windows-x64.zip'
    }
    Write-Host 'Ready: build-windows/twinleap.exe'
    if ($Run) { Invoke-Checked (Join-Path $PSScriptRoot 'build-windows\twinleap.exe') @() }
}
finally { Pop-Location }
