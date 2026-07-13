# Build AyuGram on Windows x64 from a clean machine

This guide is the canonical setup path for this fork. It works on any Windows
drive and does not require a directory named `TBuild`.

Commands containing variables, quotes, script blocks, or multiple statements
must be saved as a temporary `.ps1` file and run with `pwsh -NoProfile -File`.
Agents should create those temporary scripts with `apply_patch`, keep them
outside the repository, and remove them after verification.

## Path model

Choose a short absolute directory without spaces and call it `<BuildRoot>`.
`C:\AyuGramBuild` is an example, not a required location. The current wrappers
invoke Python scripts through unquoted batch paths, so a path containing spaces
is not supported yet.

Use the following layout:

```text
<BuildRoot>\
├── tdesktop\                 <RepoRoot>, this Git repository
│   └── out\                  <OutRoot>, generated CMake build tree
├── Libraries\win64\         prepared Windows x64 dependencies
├── ThirdParty\               prepared tools and source caches
├── Tools\gitleaks-8.30.1\   pinned local secret scanner
└── release\                  <ReleaseRoot>, packaged release assets
```

The repository must be a direct child of `<BuildRoot>`.
`Telegram/build/prepare/prepare.py` derives `<BuildRoot>` by walking upward from
its own location; moving only the repository or copying prepared dependency
folders to another layout breaks that relationship.

Never copy `out`, `Libraries`, or `ThirdParty` from a machine with a different
compiler, SDK, architecture, or source revision. Choose the path once, then
prepare and configure there.

## Security and privacy rules

The build requires `TDESKTOP_API_ID` and `TDESKTOP_API_HASH`. They are build
credentials and must never appear as literal values in Markdown, scripts under
Git, commits, issue text, terminal history, or agent output.

- Obtain the values from the user through an approved secret channel. An agent
  must stop and request them when the process environment does not contain
  both variables; it must not invent, search for, or reuse a value found in an
  older guide.
- Pass the values through process environment variables. Do not print the
  variables, run an unrestricted environment dump, or call commands that emit
  authentication tokens.
- `out/CMakeCache.txt` contains absolute paths and can contain the API fields.
  `out/` is Git-ignored, but it must still be treated as local sensitive build
  state and must not be uploaded as a diagnostic artifact.
- Never commit a Windows account name, home-directory path, hostname, private
  email, phone number, IP address, private key, signing passphrase, GitHub
  token, webhook, or password. Use `<BuildRoot>`, `<RepoRoot>`, `<fork-url>`,
  and `<tag>` placeholders in durable documents.
- The public repository owner in the clone URL is an intentional project
  identifier. Do not infer or record a person's private identity from it.

Before every commit, inspect the exact staged file list and run a fully
redacted secret scan. See [Pre-commit privacy gate](#pre-commit-privacy-gate).

## Prerequisites

Use Windows 11 x64 and Visual Studio 2022. Install these Visual Studio
components through Visual Studio Installer:

- Desktop development with C++.
- MSVC v143 x64/x86 build tools.
- C++ ATL for the current v143 tools.
- C++ MFC for the current v143 tools.
- Windows 11 SDK `10.0.26100.0`.
- CMake tools for Windows.

Install Python 3.10 with `python.exe` on `PATH`, Git for Windows, and
PowerShell 7. GitHub CLI is required only for publishing a release.

Installing Visual Studio components or machine-wide packages may require
administrator access. An agent must show the exact installation command and
its purpose, then wait for user authorization instead of elevating itself.

Open **x64 Native Tools Command Prompt for VS 2022**, then start
`pwsh -NoProfile` inside it. This preserves the MSVC, ATL, SDK, and platform
environment while using PowerShell 7 for the remaining checks.

## Toolchain preflight

Save the following as a temporary script outside the repository and run it
with `pwsh -NoProfile -File <script-path>`:

```powershell
$ErrorActionPreference = 'Stop'

$requiredCommands = @(
    'git',
    'python',
    'cmake',
    'ninja.exe',
    'cl.exe',
    'msbuild.exe',
    'pwsh'
)
$missing = @($requiredCommands | Where-Object {
    -not (Get-Command $_ -ErrorAction SilentlyContinue)
})
if ($missing.Count -ne 0) {
    throw "Missing commands: $($missing -join ', ')"
}

$pythonVersion = (& python --version 2>&1).ToString()
if ($pythonVersion -notmatch '^Python 3\.10\.') {
    throw "Expected Python 3.10, got $pythonVersion."
}
if ($env:Platform -ne 'x64') {
    throw 'Use the x64 Native Tools environment.'
}
if ($env:VisualStudioVersion -notlike '17.*') {
    throw "Expected Visual Studio 2022, got $env:VisualStudioVersion."
}
foreach ($name in @('VCToolsInstallDir', 'WindowsSDKVersion')) {
    if ([string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($name))) {
        throw "The MSVC environment variable $name is missing."
    }
}

$sdk = $env:WindowsSDKVersion.TrimEnd('\')
if ($sdk -ne '10.0.26100.0') {
    throw "Expected Windows SDK 10.0.26100.0, got $sdk."
}
$atlHeader = Join-Path $env:VCToolsInstallDir 'atlmfc\include\atlbase.h'
$mfcHeader = Join-Path $env:VCToolsInstallDir 'atlmfc\include\afxwin.h'
if (-not (Test-Path -LiteralPath $atlHeader -PathType Leaf)) {
    throw 'The Visual Studio ATL component is missing.'
}
if (-not (Test-Path -LiteralPath $mfcHeader -PathType Leaf)) {
    throw 'The Visual Studio MFC component is missing.'
}

Write-Output "Python=$pythonVersion"
Write-Output "VisualStudio=$env:VisualStudioVersion SDK=$sdk Platform=$env:Platform"
```

Do not continue when this preflight fails. Repair the missing prerequisite and
run the bounded preflight again.

## Install the pinned secret scanner

Commit checks use Gitleaks 8.30.1 from its
[official release](https://github.com/gitleaks/gitleaks/releases/tag/v8.30.1).
Install it outside the repository and verify the Windows x64 archive before
extracting it. Save this as a temporary script:

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$BuildRoot
)

$ErrorActionPreference = 'Stop'
$version = '8.30.1'
$archiveName = "gitleaks_${version}_windows_x64.zip"
$expectedSha256 =
    'D29144DEFF3A68AA93CED33DDDF84B7FDC26070ADD4AA0F4513094C8332AFC4E'
$BuildRoot = [System.IO.Path]::GetFullPath($BuildRoot)
if ($BuildRoot -match '\s') {
    throw 'BuildRoot must not contain spaces.'
}

$toolsRoot = Join-Path $BuildRoot 'Tools'
$installRoot = Join-Path $toolsRoot "gitleaks-$version"
$archivePath = Join-Path $toolsRoot $archiveName
if ((Test-Path -LiteralPath $installRoot) -or
    (Test-Path -LiteralPath $archivePath)) {
    throw 'Refusing to overwrite an existing Gitleaks path.'
}

New-Item -ItemType Directory -Path $toolsRoot -Force | Out-Null
$downloadUrl =
    "https://github.com/gitleaks/gitleaks/releases/download/v$version/$archiveName"
Invoke-WebRequest -Uri $downloadUrl -OutFile $archivePath
$archiveHash = Get-FileHash -LiteralPath $archivePath -Algorithm SHA256
$actualSha256 = $archiveHash.Hash.ToUpperInvariant()
if ($actualSha256 -ne $expectedSha256) {
    throw 'The downloaded Gitleaks archive failed SHA-256 verification.'
}

Expand-Archive -LiteralPath $archivePath -DestinationPath $installRoot
$executable = Join-Path $installRoot 'gitleaks.exe'
if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw 'The verified archive did not contain gitleaks.exe.'
}
$reportedVersion = (& $executable version).Trim()
if ($LASTEXITCODE -ne 0 -or $reportedVersion -ne $version) {
    throw 'The extracted Gitleaks version is not the pinned version.'
}

Write-Output "Gitleaks=$reportedVersion SHA256=$actualSha256"
```

Keep the verified archive as local provenance data. Do not commit the archive,
the extracted executable, or the `Tools` directory.

## Clone the fork

The normal project checkout is the user's fork on branch `dev`. A developer
may pass a different personal fork URL, but must never use an external upstream
as a writable remote.

Save this as another temporary script outside Git:

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$BuildRoot,
    [string]$ForkUrl = 'https://github.com/zeta987/AyuGramDesktop.git'
)

$ErrorActionPreference = 'Stop'
$BuildRoot = [System.IO.Path]::GetFullPath($BuildRoot)
if ($BuildRoot -match '\s') {
    throw 'BuildRoot must not contain spaces.'
}
if ($ForkUrl -match '(?i)^https?://[^/@:]+:[^/@]+@') {
    throw 'ForkUrl must not contain embedded credentials.'
}
if ($ForkUrl -match `
    '(?i)github\.com(?:/|:)(?:ayugram/ayugramdesktop|telegramdesktop/tdesktop)(?:\.git)?/?(?:[?#].*)?$') {
    throw 'ForkUrl must identify a user-owned fork, not an upstream repository.'
}
$RepoRoot = Join-Path $BuildRoot 'tdesktop'
if (Test-Path -LiteralPath $RepoRoot) {
    throw "Refusing to overwrite existing path: $RepoRoot"
}

function Assert-NoIndexLock {
    if (Test-Path -LiteralPath (Join-Path $RepoRoot '.git\index.lock')) {
        throw 'The checkout contains a Git index lock.'
    }
}

New-Item -ItemType Directory -Path $BuildRoot -Force | Out-Null
Push-Location -LiteralPath $BuildRoot
try {
    & git clone --branch dev --recursive $ForkUrl 'tdesktop'
    if ($LASTEXITCODE -ne 0) {
        throw 'Recursive clone failed.'
    }
} finally {
    Pop-Location
}
Assert-NoIndexLock
& git -C $RepoRoot remote add upstream `
    'https://github.com/AyuGram/AyuGramDesktop.git'
if ($LASTEXITCODE -ne 0) {
    throw 'Could not add the AyuGram upstream remote.'
}
Assert-NoIndexLock
& git -C $RepoRoot remote set-url --push upstream 'disabled://upstream-push'
if ($LASTEXITCODE -ne 0) {
    throw 'Could not disable upstream pushes.'
}
Assert-NoIndexLock
& git -C $RepoRoot remote add telegram `
    'https://github.com/telegramdesktop/tdesktop.git'
if ($LASTEXITCODE -ne 0) {
    throw 'Could not add the Telegram upstream remote.'
}
Assert-NoIndexLock
& git -C $RepoRoot remote set-url --push telegram 'disabled://telegram-push'
if ($LASTEXITCODE -ne 0) {
    throw 'Could not disable Telegram upstream pushes.'
}
Assert-NoIndexLock
& git -C $RepoRoot submodule sync --recursive
if ($LASTEXITCODE -ne 0) {
    throw 'Submodule URL synchronization failed.'
}
Assert-NoIndexLock
& git -C $RepoRoot submodule update --init --recursive
if ($LASTEXITCODE -ne 0) {
    throw 'Submodule initialization failed.'
}

Write-Output 'checkout-created=True'
```

After cloning, inspect `.gitmodules` and `git remote -v`. For this project,
`origin` is the only writable superproject remote. `upstream`, `telegram`, and
all external upstream repositories remain read-only. Create a fork before
modifying any submodule that does not already point to a user-owned fork.

## Prepare dependencies

Dependency preparation downloads and compiles a large toolchain. Start it only
after the user authorizes the long-running operation. Keep the computer awake
and preserve the same x64 Native Tools environment used by the preflight.

Save and run this temporary script:

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$RepoRoot
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path -LiteralPath $RepoRoot).Path
$BuildRoot = Split-Path $RepoRoot -Parent
if ($BuildRoot -match '\s') {
    throw 'BuildRoot must not contain spaces.'
}
if ($env:Platform -ne 'x64') {
    throw 'Use the x64 Native Tools environment.'
}

$prepare = Join-Path $RepoRoot 'Telegram\build\prepare\win.bat'
& $prepare silent qt6
if ($LASTEXITCODE -ne 0) {
    throw 'Dependency preparation failed.'
}
```

Do not pass `skip-release`: the local publication procedure needs both Debug
and Release dependency variants. Keep `qt6`, which selects the Qt version from
`Telegram/build/qt_version.py`.

An interrupted preparation is normally incremental. Read the first failing
stage, repair that specific prerequisite or network failure, then rerun the
same preparation command. Do not delete all prepared dependencies as a first
response.

## Configure the project

Before launching the configure script, the parent PowerShell process must
already contain non-empty `TDESKTOP_API_ID` and `TDESKTOP_API_HASH` variables.
The user or an approved secret manager supplies them without exposing their
values to the agent transcript.

Save and run this temporary script:

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$RepoRoot
)

$ErrorActionPreference = 'Stop'
if ([string]::IsNullOrWhiteSpace($env:TDESKTOP_API_ID) -or
    [string]::IsNullOrWhiteSpace($env:TDESKTOP_API_HASH)) {
    throw 'TDESKTOP_API_ID and TDESKTOP_API_HASH must be supplied securely.'
}

$RepoRoot = (Resolve-Path -LiteralPath $RepoRoot).Path
$telegramRoot = Join-Path $RepoRoot 'Telegram'
Push-Location -LiteralPath $telegramRoot
try {
    & '.\configure.bat' `
        qt6 `
        -G 'Visual Studio 17 2022' `
        -Ax64 `
        "-DTDESKTOP_API_ID=$env:TDESKTOP_API_ID" `
        "-DTDESKTOP_API_HASH=$env:TDESKTOP_API_HASH" `
        -DCMAKE_COMPILE_WARNING_AS_ERROR=ON `
        -DDESKTOP_APP_ENABLE_LTO=OFF `
        -DDESKTOP_APP_DISABLE_AUTOUPDATE=OFF `
        -DDESKTOP_APP_DISABLE_CRASH_REPORTS=OFF
    if ($LASTEXITCODE -ne 0) {
        throw 'CMake configuration failed.'
    }
} finally {
    Pop-Location
}
```

The explicit generator prevents a newer side-by-side Visual Studio install
from silently selecting another toolchain. `-Ax64` is passed as one argument
because the repository wrapper otherwise interprets a standalone `x64`
argument as its default-generator shortcut.

This fork's current local release reference keeps LTO off, matching
`v6.9.4-beta.7`. Auto-update and crash-report support remain enabled. Changing
those options creates a different publication profile and requires a separate
review.

After configuration, inspect `<RepoRoot>\out\CMakeCache.txt` and verify the
generator, architecture, Qt path, SDK, and three `DESKTOP_APP_*` options. Never
paste the API entries from that cache into an issue or chat.

## Build Debug

Save the command in a temporary script so the path is handled as one argument:

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$RepoRoot
)

$ErrorActionPreference = 'Stop'
$OutRoot = Join-Path (Resolve-Path -LiteralPath $RepoRoot).Path 'out'
& cmake --build $OutRoot --config Debug --target Telegram --parallel 4
if ($LASTEXITCODE -ne 0) {
    throw 'Debug build failed.'
}
```

The expected executables are:

```text
<RepoRoot>\out\Debug\AyuGram.exe
<RepoRoot>\out\Debug\Updater.exe
```

Record the source commit, FileVersion, ProductVersion, SHA-256, exact test
steps, and whether the developer accepted the build. Do not build Release,
create a tag, or upload assets before that confirmation.

## Build and package Release

After the developer accepts Debug, create and verify signed commits, integrate
them into a clean `dev`, and make every custom submodule commit available from
the corresponding user-owned fork. Then build Release from the same
`<RepoRoot>\out` tree:

```powershell
param(
    [Parameter(Mandatory = $true)]
    [string]$RepoRoot
)

$ErrorActionPreference = 'Stop'
$OutRoot = Join-Path (Resolve-Path -LiteralPath $RepoRoot).Path 'out'
& cmake --build $OutRoot --config Release --target Telegram --parallel 4
if ($LASTEXITCODE -ne 0) {
    throw 'Release build failed.'
}
```

Use `docs/ayugram-local-development.md` for version checks, signed tags,
packaging, asset digests, and creation of a normal GitHub Release. The packager
accepts explicit portable paths:

```powershell
pwsh -NoProfile -File <RepoRoot>\Telegram\build\package_windows_release.ps1 `
    -Tag <tag> `
    -RepositoryRoot <RepoRoot> `
    -OutputDirectory <ReleaseRoot>
```

Angle-bracket names in the example are placeholders. Replace them inside a
temporary `.ps1` file; do not paste the block unchanged into PowerShell.

## Pre-commit privacy gate

Before every commit, check `.git/index.lock`, stage only intended files, and
review their names. Never include `out`, CMake caches, temporary scripts, log
files, credential exports, or local release assets.

Run the pinned, SHA-256-verified scanner installed earlier in this guide:

```powershell
& <BuildRoot>\Tools\gitleaks-8.30.1\gitleaks.exe git `
    --staged --redact=100 --no-banner --no-color <RepoRoot>
```

Then manually inspect the staged documentation for private emails, Windows
profile paths, hostnames, IP addresses, phone numbers, real names, and literal
credential assignments. Scanner success alone cannot identify every form of
personal data.

Finish with `git diff --cached --check`, inspect `git diff --cached`, create a
signed commit, and run `git verify-commit HEAD`. Do not print a discovered
secret while reporting it; identify only its category, file, and line, then
remove or rotate it through an approved channel.

## Troubleshooting

### Wrong command prompt

If `Platform`, `VisualStudioVersion`, `VCToolsInstallDir`, or
`WindowsSDKVersion` is missing, close the shell and reopen x64 Native Tools
Command Prompt for Visual Studio 2022. Do not retry preparation in an ordinary
terminal.

### Unsupported path

If a wrapper reports that it cannot find `prepare.py` or `configure.py`, verify
that `<BuildRoot>` is a short absolute path without spaces and `<RepoRoot>` is
its direct child. Do not create a fixed `D:\TBuild` directory unless the user
actually selected that location.

### PDB or EXE access failure

Stop after `C1041`, `LNK1104`, access-denied, or file-in-use errors. Ask the
user to close `AyuGram.exe` and attached debuggers, confirm the process has
ended, then retry once. Do not delete locked outputs.

### PDB configuration failure

The `/FS` and release debug-information fixes are already present in this
fork's `cmake` submodule. Update the customized submodule from its user-owned
fork and verify its commit instead of applying the obsolete inline patch from
older build notes.

### Cache from another machine

`out/CMakeCache.txt` stores absolute paths and toolchain details. A copied or
moved cache is not portable. Preserve the old directory for diagnosis, create
a fresh BuildRoot selected by the user, and repeat prepare and configure there.
