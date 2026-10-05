[CmdletBinding()]
param([string]$DllPath)
$ErrorActionPreference = 'Stop'
if (-not $DllPath) { $DllPath = Join-Path (Split-Path -Parent $PSScriptRoot) 'build\release\Release\SkyrimsDogma.dll' }
# Inspect the PE file as bytes. Never load or execute it outside Skyrim.
$binary = [System.IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $DllPath).Path)
function Read-U16([int]$Offset) {
    if ($Offset -lt 0 -or $Offset + 2 -gt $binary.Length) { throw 'Truncated PE data.' }
    return [BitConverter]::ToUInt16($binary, $Offset)
}
function Read-U32([int]$Offset) {
    if ($Offset -lt 0 -or $Offset + 4 -gt $binary.Length) { throw 'Truncated PE data.' }
    return [BitConverter]::ToUInt32($binary, $Offset)
}
if ((Read-U16 0) -ne 0x5A4D) { throw 'Not a Windows executable.' }
$pe = Read-U32 0x3C
if ((Read-U32 $pe) -ne 0x4550 -or (Read-U16 ($pe + 4)) -ne 0x8664) { throw 'Expected an x64 PE file.' }
if (((Read-U16 ($pe + 22)) -band 0x2000) -eq 0) { throw 'PE file is not marked as a DLL.' }
$optional = $pe + 24
if ((Read-U16 $optional) -ne 0x20B) { throw 'Expected a PE32+ optional header.' }
$sectionCount = Read-U16 ($pe + 6)
$sectionStart = $optional + (Read-U16 ($pe + 20))
$headerSize = Read-U32 ($optional + 60)
$sections = @()
for ($index = 0; $index -lt $sectionCount; ++$index) {
    $entry = $sectionStart + 40 * $index
    $sections += [pscustomobject]@{
        RVA = Read-U32 ($entry + 12)
        Size = Read-U32 ($entry + 16)
        FileOffset = Read-U32 ($entry + 20)
    }
}
function Resolve-RVA([uint32]$RVA) {
    if ($RVA -lt $headerSize -and $RVA -lt $binary.Length) { return [int]$RVA }
    foreach ($section in $sections) {
        if ($RVA -ge $section.RVA -and $RVA -lt ([uint64]$section.RVA + $section.Size)) {
            $offset = [int]($section.FileOffset + $RVA - $section.RVA)
            if ($offset -ge $binary.Length) { throw 'PE address lies outside the file.' }
            return $offset
        }
    }
    throw "Cannot resolve PE address $RVA."
}
function Read-ASCII([int]$Offset) {
    $end = $Offset
    while ($end -lt $binary.Length -and $binary[$end] -ne 0 -and $end - $Offset -lt 512) { ++$end }
    if ($end -ge $binary.Length -or $binary[$end] -ne 0) { throw 'Invalid PE export name.' }
    return [System.Text.Encoding]::ASCII.GetString($binary, $Offset, $end - $Offset)
}
$exportRVA = Read-U32 ($optional + 112)
if ($exportRVA -eq 0) { throw 'No export table.' }
$exportDirectory = Resolve-RVA $exportRVA
$nameCount = Read-U32 ($exportDirectory + 24)
if ($nameCount -gt 10000) { throw 'Unexpected export table size.' }
$functions = Resolve-RVA (Read-U32 ($exportDirectory + 28))
$names = Resolve-RVA (Read-U32 ($exportDirectory + 32))
$ordinals = Resolve-RVA (Read-U32 ($exportDirectory + 36))
$exports = @{}
for ($index = 0; $index -lt $nameCount; ++$index) {
    $name = Read-ASCII (Resolve-RVA (Read-U32 ($names + 4 * $index)))
    $ordinal = Read-U16 ($ordinals + 2 * $index)
    $exports[$name] = Resolve-RVA (Read-U32 ($functions + 4 * $ordinal))
}
foreach ($required in @('SKSEPlugin_Load', 'SKSEPlugin_Query', 'SKSEPlugin_Version')) {
    if (-not $exports.ContainsKey($required)) { throw "Missing SKSE export: $required" }
}

$metadata = $exports['SKSEPlugin_Version']
if ((Read-U32 $metadata) -ne 1) { throw 'Unexpected SKSE metadata format.' }
if ((Read-U32 ($metadata + 4)) -ne 0x00010010) { throw 'Expected plugin version 0.1.1.' }
if ((Read-ASCII ($metadata + 8)) -ne 'SkyrimsDogma') { throw 'Unexpected plugin name.' }
if (((Read-U32 ($metadata + 0x304)) -band 2) -eq 0) { throw 'Address Library format-5 compatibility bit missing.' }
if ((Read-U32 ($metadata + 0x308)) -ne 0) { throw 'Version-independent flag would bypass the exact runtime whitelist.' }
if ((Read-U32 ($metadata + 0x30C)) -ne 0x01070680) { throw 'Runtime whitelist does not target 1.7.104.0.' }
for ($index = 1; $index -lt 16; ++$index) {
    if ((Read-U32 ($metadata + 0x30C + 4 * $index)) -ne 0) { throw 'An additional runtime is advertised unexpectedly.' }
}
if ((Read-U32 ($metadata + 0x34C)) -ne 0x02030010) { throw 'Expected minimum SKSE version 2.3.1.' }
[pscustomobject]@{
    Machine = 'x64'
    Exports = ($exports.Keys | Sort-Object) -join ', '
    PluginVersion = '0.1.1'
    ExactRuntime = '1.7.104.0'
    MinimumSKSE = '2.3.1'
    AddressLibraryV5 = $true
    SHA256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $DllPath).Hash
    InGameBehavior = 'not tested by this static check'
} | Format-List
