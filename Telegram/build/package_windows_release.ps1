param(
    [Parameter(Mandatory = $true)]
    [string]$Tag,
    [string]$RepositoryRoot,
    [string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

if ([string]::IsNullOrWhiteSpace($RepositoryRoot)) {
    $RepositoryRoot = Join-Path $PSScriptRoot '..\..'
}
$RepositoryRoot = (Resolve-Path -LiteralPath $RepositoryRoot).Path
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path (
        Split-Path $RepositoryRoot -Parent) 'release'
}
$OutputDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)

function Invoke-Git {
    param([Parameter(Mandatory = $true)][string[]]$Arguments)

    $output = & git -C $RepositoryRoot @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed:`n$($output -join "`n")"
    }
    return @($output)
}

$tagMatch = [regex]::Match(
    $Tag,
    '^v(?<version>\d+\.\d+\.\d+)(?<suffix>-beta\.\d+)?$')
if (-not $tagMatch.Success) {
    throw "Unsupported release tag: $Tag"
}

$version = $tagMatch.Groups['version'].Value
$isBeta = $tagMatch.Groups['suffix'].Success
$expectedFileVersion = "$version.0"
$versionFile = Join-Path $RepositoryRoot 'Telegram\build\version'
$versionValues = @{}
foreach ($line in Get-Content -LiteralPath $versionFile) {
    if ($line -match '^(?<name>\w+)\s+(?<value>\S+)$') {
        $versionValues[$Matches.name] = $Matches.value
    }
}
if ($versionValues.AppVersionStr -ne $version) {
    throw "AppVersionStr $($versionValues.AppVersionStr) does not match $version."
}
if ($versionValues.AlphaVersion -ne '0') {
    throw 'Release packaging requires AlphaVersion=0.'
}
if ($isBeta) {
    if ($versionValues.BetaChannel -ne '1' -or
        $versionValues.AppVersionOriginal -ne "$version.beta") {
        throw 'Beta packaging requires BetaChannel=1 and AppVersionOriginal=<version>.beta.'
    }
} elseif ($versionValues.BetaChannel -ne '0' -or
    $versionValues.AppVersionOriginal -ne $version) {
    throw 'Stable packaging requires BetaChannel=0 and an exact AppVersionOriginal.'
}

$status = @(Invoke-Git -Arguments @('status', '--porcelain'))
if ($status.Count -ne 0) {
    throw "The repository is not clean:`n$($status -join "`n")"
}
$head = @(Invoke-Git -Arguments @('rev-parse', 'HEAD'))[0].Trim()
$tagType = @(Invoke-Git -Arguments @('cat-file', '-t', $Tag))[0].Trim()
if ($tagType -ne 'tag') {
    throw "$Tag must be an annotated tag."
}
[void](Invoke-Git -Arguments @('verify-commit', 'HEAD'))
[void](Invoke-Git -Arguments @('verify-tag', $Tag))
$tagCommit = @(Invoke-Git -Arguments @('rev-list', '-n', '1', $Tag))[0].Trim()
if ($tagCommit -ne $head) {
    throw "Tag $Tag points to $tagCommit instead of HEAD $head."
}

$releaseDirectory = Join-Path $RepositoryRoot 'out\Release'
$binaryPaths = [ordered]@{
    'AyuGram.exe' = Join-Path $releaseDirectory 'AyuGram.exe'
    'Updater.exe' = Join-Path $releaseDirectory 'Updater.exe'
}
$ayuGram = Get-Item -LiteralPath $binaryPaths['AyuGram.exe'] `
    -ErrorAction Stop
$ayuGramFileVersion = [string]$ayuGram.VersionInfo.FileVersion
$ayuGramProductVersion = [string]$ayuGram.VersionInfo.ProductVersion
if ($ayuGramFileVersion -ne $expectedFileVersion -or
    $ayuGramProductVersion -ne $expectedFileVersion) {
    throw "AyuGram.exe version $ayuGramFileVersion/$ayuGramProductVersion does not match $expectedFileVersion."
}
$updater = Get-Item -LiteralPath $binaryPaths['Updater.exe'] -ErrorAction Stop
if ($updater.Length -le 0) {
    throw 'Updater.exe is empty.'
}

$modulePath = Join-Path $releaseDirectory `
    'modules\x64\d3d\d3dcompiler_47.dll'
if (-not (Test-Path -LiteralPath $modulePath -PathType Leaf)) {
    $modulePath = Join-Path $RepositoryRoot `
        'cmake\win_directx_helper\modules\x64\d3d\d3dcompiler_47.dll'
}
$module = Get-Item -LiteralPath $modulePath -ErrorAction Stop
$moduleVersionInfo = $module.VersionInfo
$moduleVersion = '{0}.{1}.{2}.{3}' -f @(
    $moduleVersionInfo.FileMajorPart,
    $moduleVersionInfo.FileMinorPart,
    $moduleVersionInfo.FileBuildPart,
    $moduleVersionInfo.FilePrivatePart
)
if ($moduleVersion -ne '10.0.22621.3233') {
    throw "Unexpected d3dcompiler_47.dll version: $moduleVersion"
}
$moduleHash = (Get-FileHash -LiteralPath $modulePath -Algorithm SHA256).Hash
if ($moduleHash -ne
    'A05D04A270F68C8C6D6EA2D23BEBF8CD1D5453B26B5442FA54965F90F1C62082') {
    throw "Unexpected d3dcompiler_47.dll SHA-256: $moduleHash"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$assetName = "AyuGram-Windows-x64-$Tag"
$zipPath = Join-Path $OutputDirectory "$assetName.zip"
$hashPath = "$zipPath.sha256"
$infoPath = Join-Path $OutputDirectory "$assetName-BUILD-INFO.txt"
foreach ($path in @($zipPath, $hashPath, $infoPath)) {
    if (Test-Path -LiteralPath $path) {
        throw "Refusing to overwrite existing release asset: $path"
    }
}

try {
    $stage = [System.IO.Path]::GetFullPath((Join-Path $OutputDirectory (
        ".stage-$([guid]::NewGuid().ToString('N'))")))
    $outputPrefix = $OutputDirectory.TrimEnd(
        [System.IO.Path]::DirectorySeparatorChar,
        [System.IO.Path]::AltDirectorySeparatorChar
    ) + [System.IO.Path]::DirectorySeparatorChar
    if (-not $stage.StartsWith(
        $outputPrefix,
        [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Unsafe staging path: $stage"
    }
    try {
        $stageModuleDirectory = Join-Path $stage 'modules\x64\d3d'
        New-Item -ItemType Directory -Path $stageModuleDirectory -Force |
            Out-Null
        foreach ($entry in $binaryPaths.GetEnumerator()) {
            Copy-Item -LiteralPath $entry.Value -Destination (
                Join-Path $stage $entry.Key)
        }
        Copy-Item -LiteralPath $modulePath -Destination $stageModuleDirectory
        Compress-Archive -Path (Join-Path $stage '*') `
            -DestinationPath $zipPath -CompressionLevel Optimal
    } finally {
        if (Test-Path -LiteralPath $stage) {
            Remove-Item -LiteralPath $stage -Recurse -Force
        }
    }

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $archive = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
    try {
        $actualEntries = @(
            $archive.Entries |
                Where-Object { -not [string]::IsNullOrEmpty($_.Name) } |
                ForEach-Object { $_.FullName.Replace('\', '/') } |
                Sort-Object
        )
    } finally {
        $archive.Dispose()
    }
    $expectedEntries = @(
        'AyuGram.exe'
        'Updater.exe'
        'modules/x64/d3d/d3dcompiler_47.dll'
    ) | Sort-Object
    $entryDifference = @(Compare-Object $expectedEntries $actualEntries)
    if ($entryDifference.Count -ne 0) {
        throw "Unexpected ZIP entries:`n$($entryDifference | Out-String)"
    }

    $zip = Get-Item -LiteralPath $zipPath
    if ($zip.Length -ge 2GB) {
        throw 'The ZIP exceeds GitHub release asset limit of 2 GiB.'
    }
    $hash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).
        Hash.ToLowerInvariant()
    "$hash  $($zip.Name)" |
        Set-Content -LiteralPath $hashPath -Encoding ascii

    $cacheValues = @{}
    $cachePath = Join-Path $RepositoryRoot 'out\CMakeCache.txt'
    foreach ($line in Get-Content -LiteralPath $cachePath) {
        if ($line -match '^(?<name>[^:#]+):[^=]+=(?<value>.*)$') {
            $cacheValues[$Matches.name] = $Matches.value
        }
    }
    $cmakeVersion = 'unknown'
    $cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($null -ne $cmakeCommand) {
        $cmakeLines = @(& $cmakeCommand.Source --version)
        if ($cmakeLines.Count -ne 0) {
            $cmakeVersion = [string]$cmakeLines[0]
        }
    }
    $compilerVersion = 'unknown'
    $compilerMetadata = Get-ChildItem -LiteralPath (
        Join-Path $RepositoryRoot 'out\CMakeFiles') `
        -Filter CMakeCXXCompiler.cmake -File -Recurse |
        Sort-Object LastWriteTimeUtc -Descending |
        Select-Object -First 1
    if ($null -ne $compilerMetadata) {
        $compilerContents = Get-Content -LiteralPath (
            $compilerMetadata.FullName) -Raw
        if ($compilerContents -match
            'CMAKE_CXX_COMPILER_VERSION\s+"(?<version>[^"]+)"') {
            $compilerVersion = $Matches.version
        }
    }
    $qtVersion = 'unknown'
    if ([string]$cacheValues['Qt6Core_DIR'] -match
        'Qt-(?<version>\d+\.\d+\.\d+)') {
        $qtVersion = $Matches.version
    }

    $ayuSignature = Get-AuthenticodeSignature -LiteralPath $ayuGram.FullName
    $updaterSignature = Get-AuthenticodeSignature -LiteralPath $updater.FullName
    $ayuGramHash = (
        Get-FileHash -LiteralPath $ayuGram.FullName -Algorithm SHA256).Hash
    $updaterHash = (
        Get-FileHash -LiteralPath $updater.FullName -Algorithm SHA256).Hash
    $buildInfo = @(
        "Commit=$head"
        "ReleaseTag=$Tag"
        'Configuration=Release'
        "ExpectedFileVersion=$expectedFileVersion"
        "AyuGramFileVersion=$($ayuGram.VersionInfo.FileVersion)"
        "UpdaterFileVersion=$($updater.VersionInfo.FileVersion)"
        "AyuGramBuildTimeUtc=$($ayuGram.LastWriteTimeUtc.ToString('o'))"
        "UpdaterBuildTimeUtc=$($updater.LastWriteTimeUtc.ToString('o'))"
        "AyuGramAuthenticode=$($ayuSignature.Status)"
        "UpdaterAuthenticode=$($updaterSignature.Status)"
        "AyuGramSha256=$ayuGramHash"
        "UpdaterSha256=$updaterHash"
        "D3DCompilerSha256=$moduleHash"
        "CMake=$cmakeVersion"
        "CMakeGenerator=$([string]$cacheValues['CMAKE_GENERATOR'])"
        "MSVC=$compilerVersion"
        "Qt=$qtVersion"
        "ArchiveSha256=$hash"
    )
    $buildInfo | Set-Content -LiteralPath $infoPath -Encoding ascii

    [pscustomobject]@{
        Tag = $Tag
        Commit = $head
        Zip = $zipPath
        Sha256File = $hashPath
        BuildInfo = $infoPath
        ZipBytes = $zip.Length
        Sha256 = $hash
    }
} catch {
    foreach ($path in @($zipPath, $hashPath, $infoPath)) {
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            Remove-Item -LiteralPath $path -Force
        }
    }
    throw
}
