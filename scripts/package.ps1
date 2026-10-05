[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
function New-PortableZip([string]$Directory, [string]$ArchivePath) {
    $root = (Resolve-Path -LiteralPath $Directory).Path.TrimEnd('\') + '\'
    $outputPath = [System.IO.Path]::GetFullPath($ArchivePath)
    $stream = [System.IO.File]::Open($outputPath, [System.IO.FileMode]::Create)
    $zip = New-Object System.IO.Compression.ZipArchive($stream, [System.IO.Compression.ZipArchiveMode]::Create, $false)
    try {
        Get-ChildItem -LiteralPath $Directory -File -Recurse -Force | ForEach-Object {
            if (-not $_.FullName.StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)) { throw 'File outside package staging directory.' }
            $relative = $_.FullName.Substring($root.Length).Replace('\', '/')
            [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $_.FullName, $relative, [System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
        }
    } finally {
        $zip.Dispose()
        $stream.Dispose()
    }
}
$dll = Join-Path $projectRoot 'build\release\Release\SkyrimsDogma.dll'
if (-not (Test-Path -LiteralPath $dll)) { throw 'Run scripts/build.ps1 successfully first.' }
& (Join-Path $PSScriptRoot 'verify-artifact.ps1') -DllPath $dll

# A fresh staging directory avoids stale files without deleting any previous package.
$stage = Join-Path $projectRoot ('build\package-' + [guid]::NewGuid().ToString('N'))
$plugins = Join-Path $stage 'SKSE\Plugins'
$notes = Join-Path $stage 'SkyrimsDogma\notices'
$packagedDocs = Join-Path $stage 'SkyrimsDogma\docs'
New-Item -ItemType Directory -Force -Path $plugins,$notes,$packagedDocs,'dist' | Out-Null
Copy-Item -LiteralPath $dll -Destination $plugins
Copy-Item -LiteralPath 'config\SkyrimsDogma.ini' -Destination $plugins
Copy-Item -LiteralPath 'README.md','DESIGN.md','STATUS.md','AGENTS.md','LICENSE','LICENSE-EXCEPTIONS.md','THIRD_PARTY.md' -Destination (Join-Path $stage 'SkyrimsDogma')
Get-ChildItem -LiteralPath 'docs' -Filter '*.md' -File | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $packagedDocs
}
Copy-Item -LiteralPath '.deps\CommonLibSSE-NG\COPYING.txt' -Destination (Join-Path $notes 'CommonLib-GPL.txt')
Copy-Item -LiteralPath '.deps\CommonLibSSE-NG\EXCEPTIONS.md' -Destination (Join-Path $notes 'CommonLib-exceptions.md')
Copy-Item -LiteralPath '.deps\CommonLibSSE-NG\licenses\LICENSE-MIT.txt' -Destination (Join-Path $notes 'CommonLib-original-MIT.txt')
Copy-Item -LiteralPath '.deps\CommonLibSSE-NG\licenses\LICENSE-hde64.txt' -Destination (Join-Path $notes 'hde64.txt')
$dependencyNotices = Get-ChildItem -LiteralPath 'build\release\vcpkg_installed\x64-windows-static-md\share' -Filter copyright -Recurse -File
foreach ($notice in $dependencyNotices) {
    Copy-Item -LiteralPath $notice.FullName -Destination (Join-Path $notes ($notice.Directory.Name + '.txt'))
}
$openvrNotice = '.deps\CommonLibSSE-NG\extern\openvr\LICENSE'
if (Test-Path -LiteralPath $openvrNotice) {
    Copy-Item -LiteralPath $openvrNotice -Destination (Join-Path $notes 'OpenVR.txt')
}
$manifest = [ordered]@{
    plugin = 'SkyrimsDogma'
    version = '0.1.1-observer'
    targetRuntime = '1.7.104.0'
    minimumSKSE = '2.3.1'
    gameplayWrites = $false
    packagedUtc = [DateTime]::UtcNow.ToString('o')
    sourceBaseCommit = (& git rev-parse HEAD)
    sourceContainsUncommittedChanges = [bool](& git status --porcelain)
    commonLibCommit = '94faaed0c60eddd8347767f2d4d29a97c93bde8c'
    vcpkgBaseline = 'ee12231b20c95013c6638d845d04c91559a1d1ff'
    dllSHA256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $dll).Hash
    liveGameTesting = 'not tested'
}
$manifest | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $stage 'SkyrimsDogma\build-manifest.json') -Encoding UTF8

$archive = Join-Path $projectRoot 'dist\SkyrimsDogma-0.1.1-observer.zip'
New-PortableZip $stage $archive
$pdb = Join-Path $projectRoot 'build\release\Release\SkyrimsDogma.pdb'
if (Test-Path -LiteralPath $pdb) { Copy-Item -LiteralPath $pdb -Destination 'dist\SkyrimsDogma-0.1.1.pdb' -Force }

# Snapshot the exact original source, including authorized but uncommitted work.
# git's whitelist keeps binaries, local paths/data, and dependency checkouts out.
$sourceStage = Join-Path $stage 'source-snapshot'
New-Item -ItemType Directory -Path $sourceStage | Out-Null
$sourceFiles = & git -c core.quotepath=false ls-files --cached --others --exclude-standard
if ($LASTEXITCODE -ne 0) { throw 'Cannot inventory source snapshot.' }
foreach ($relative in $sourceFiles) {
    # A tracked file may have been deleted or moved in the uncommitted worktree.
    if (-not (Test-Path -LiteralPath (Join-Path $projectRoot $relative) -PathType Leaf)) { continue }
    $target = Join-Path $sourceStage $relative
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $target) | Out-Null
    Copy-Item -LiteralPath (Join-Path $projectRoot $relative) -Destination $target
}
New-PortableZip $sourceStage 'dist\SkyrimsDogma-0.1.1-source.zip'
Get-FileHash -Algorithm SHA256 -LiteralPath $archive,'dist\SkyrimsDogma-0.1.1-source.zip' |
    ForEach-Object { $_.Hash + '  ' + [System.IO.Path]::GetFileName($_.Path) } |
    Set-Content -LiteralPath 'dist\SHA256SUMS.txt' -Encoding ASCII
Get-FileHash -Algorithm SHA256 -LiteralPath $archive,$dll | Format-Table -AutoSize
Write-Host "Package ready: $archive"
Write-Host 'Install the observer ZIP through Vortex. Do not install the PDB or source ZIP.'
