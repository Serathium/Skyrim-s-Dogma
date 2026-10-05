[CmdletBinding()]
param([string]$GamePath = 'C:\Program Files (x86)\Steam\steamapps\common\Skyrim Special Edition')
$ErrorActionPreference = 'Stop'
$gameRoot = (Resolve-Path -LiteralPath $GamePath).Path
$exe = Get-Item -LiteralPath (Join-Path $gameRoot 'SkyrimSE.exe')
$version = $exe.VersionInfo.FileVersion
if ($version -ne '1.7.104.0') { throw "Unsupported Skyrim runtime: $version. This prototype targets 1.7.104.0." }
foreach ($required in @('skse64_loader.exe', 'skse64_1_7_104.dll')) {
    if (-not (Test-Path -LiteralPath (Join-Path $gameRoot $required))) { throw "Missing $required" }
}
$loader = Get-Item -LiteralPath (Join-Path $gameRoot 'skse64_loader.exe')
$database = Join-Path $gameRoot 'Data\SKSE\Plugins\versionlib-1-7-104-0.bin'
$stream = [System.IO.File]::OpenRead($database)
try {
    $header = New-Object byte[] 4
    if ($stream.Read($header, 0, 4) -ne 4) { throw 'Address Library header is truncated.' }
    $format = [BitConverter]::ToInt32($header, 0)
    if ($format -ne 5) { throw "Unexpected Address Library format: $format" }
} finally {
    $stream.Dispose()
}
[pscustomobject]@{
    SkyrimRuntime = $version
    LoaderFileVersion = $loader.VersionInfo.FileVersion
    AddressLibraryFormat = $format
    ExecutableSHA256 = (Get-FileHash -LiteralPath $exe.FullName -Algorithm SHA256).Hash
    LivePluginLoad = 'not tested; this check only reads installed files'
} | Format-List
