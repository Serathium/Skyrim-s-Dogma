[CmdletBinding()]
param([switch]$InstallBuildTools)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = if (Test-Path -LiteralPath $vswhere) {
    & $vswhere -latest -products * -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}
if (-not $vsPath) {
    if (-not $InstallBuildTools) {
        throw 'MSVC 2022 is missing. Run .\scripts\setup.ps1 -InstallBuildTools to install Microsoft C++ build tools.'
    }
    & winget install --id Microsoft.VisualStudio.2022.BuildTools --exact --source winget --accept-source-agreements --accept-package-agreements --silent --override '--wait --quiet --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
    if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne 3010) { throw 'Build Tools installation failed.' }
}

function Get-PinnedRepository([string]$Url, [string]$Destination, [string]$Commit) {
    if (-not (Test-Path -LiteralPath (Join-Path $Destination '.git'))) {
        & git init $Destination
        if ($LASTEXITCODE -ne 0) { throw "Cannot initialize $Destination" }
        & git -C $Destination remote add origin $Url
        if ($LASTEXITCODE -ne 0) { throw "Cannot configure $Destination" }
    }
    $remote = & git -C $Destination remote get-url origin
    if ($remote -ne $Url) { throw "Unexpected remote in $Destination; leaving it untouched." }
    $dirty = & git -C $Destination status --porcelain
    if ($dirty) { throw "Local changes in $Destination; leaving them untouched." }
    & git -C $Destination fetch --depth 1 origin $Commit
    if ($LASTEXITCODE -ne 0) { throw "Cannot fetch pinned source for $Destination" }
    & git -C $Destination checkout --detach $Commit
    if ($LASTEXITCODE -ne 0) { throw "Cannot check out $Commit" }
    $actual = & git -C $Destination rev-parse HEAD
    if ($actual -ne $Commit) { throw "Source revision mismatch in $Destination" }
}

New-Item -ItemType Directory -Force -Path '.tools', '.deps', '.cache\vcpkg' | Out-Null
Get-PinnedRepository 'https://github.com/microsoft/vcpkg.git' '.tools\vcpkg' 'ee12231b20c95013c6638d845d04c91559a1d1ff'
$env:VCPKG_DISABLE_METRICS = '1'
& '.\.tools\vcpkg\bootstrap-vcpkg.bat' -disableMetrics
if ($LASTEXITCODE -ne 0) { throw 'vcpkg bootstrap failed.' }
Get-PinnedRepository 'https://github.com/alandtse/CommonLibSSE-NG.git' '.deps\CommonLibSSE-NG' '94faaed0c60eddd8347767f2d4d29a97c93bde8c'
& git -C '.deps\CommonLibSSE-NG' submodule update --init --recursive --depth 1
if ($LASTEXITCODE -ne 0) { throw 'Cannot fetch CommonLib pinned submodules.' }
Write-Host 'Pinned source dependencies are ready. Run .\scripts\build.ps1 next.'
