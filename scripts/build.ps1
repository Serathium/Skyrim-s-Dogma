[CmdletBinding()]
param([switch]$CoreOnly)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Run .\scripts\setup.ps1 -InstallBuildTools first.' }
$vsPath = & $vswhere -latest -products * -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw 'The Visual Studio 2022 C++ tools are not installed.' }
$cmake = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$ctest = Join-Path (Split-Path -Parent $cmake) 'ctest.exe'
if (-not (Test-Path -LiteralPath $cmake)) { throw 'Install the C++ CMake tools component in Visual Studio Build Tools.' }
$env:VCPKG_DISABLE_METRICS = '1'
$env:VCPKG_DEFAULT_BINARY_CACHE = Join-Path $projectRoot '.cache\vcpkg'
New-Item -ItemType Directory -Force -Path $env:VCPKG_DEFAULT_BINARY_CACHE | Out-Null
$buildDir = if ($CoreOnly) { 'build/core' } else { 'build/release' }
$configure = @('-S', '.', '-B', $buildDir, '-G', 'Visual Studio 17 2022', '-A', 'x64')
if ($CoreOnly) {
    $configure += '-DDOGMA_BUILD_PLUGIN=OFF'
} else {
    if (-not (Test-Path '.tools/vcpkg/vcpkg.exe')) { throw 'Run .\scripts\setup.ps1 first.' }
    $configure += @('-DDOGMA_BUILD_PLUGIN=ON', '-DCMAKE_TOOLCHAIN_FILE=.tools/vcpkg/scripts/buildsystems/vcpkg.cmake', '-DVCPKG_TARGET_TRIPLET=x64-windows-static-md', '-DVCPKG_HOST_TRIPLET=x64-windows-static-md')
}
& $cmake @configure
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
& $cmake --build $buildDir --config Release --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
& $ctest --test-dir $buildDir -C Release --output-on-failure --no-tests=error
if ($LASTEXITCODE -ne 0) { throw 'Tests failed.' }
Write-Host 'Build and tests passed.'
