param(
    [string]$BuildDirectory = "build-windows",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Architecture = "x64",
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Release",
    [ValidateRange(1, 128)]
    [int]$BuildParallelism = 4,
    [switch]$Launch,
    [switch]$Package,
    [string]$PackageDirectory = "dist"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$BuildPath = [System.IO.Path]::GetFullPath((Join-Path $ProjectRoot $BuildDirectory))
$ProjectPrefix = $ProjectRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar) +
    [System.IO.Path]::DirectorySeparatorChar

if (-not $BuildPath.StartsWith($ProjectPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "BuildDirectory must resolve inside the project directory: $BuildPath"
}

$CMakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
$CMakeExecutable = if ($CMakeCommand) { $CMakeCommand.Source } else { $null }
if (-not $CMakeExecutable) {
    $CMakeCandidates = @()
    $CachePath = Join-Path $BuildPath "CMakeCache.txt"
    if (Test-Path -LiteralPath $CachePath -PathType Leaf) {
        $CacheCommand = Select-String -LiteralPath $CachePath -Pattern '^CMAKE_COMMAND:INTERNAL=(.+)$' |
            Select-Object -First 1
        if ($CacheCommand) { $CMakeCandidates += $CacheCommand.Matches[0].Groups[1].Value }
    }
    if ($env:ProgramFiles) {
        $CMakeCandidates += Join-Path $env:ProgramFiles "CMake/bin/cmake.exe"
        $CMakeCandidates += Join-Path $env:ProgramFiles "Qt/Tools/CMake_64/bin/cmake.exe"
    }
    if ($env:SystemDrive) {
        $CMakeCandidates += Join-Path $env:SystemDrive "Qt/Tools/CMake_64/bin/cmake.exe"
    }
    $CMakeExecutable = $CMakeCandidates |
        Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
        Select-Object -First 1
}
if (-not $CMakeExecutable) {
    throw "CMake 3.24 or newer is required. Install it or add it to PATH."
}
$CMakeVersionOutput = & $CMakeExecutable --version
$CMakeVersionMatch = [regex]::Match(($CMakeVersionOutput -join "`n"), 'version\s+(\d+\.\d+\.\d+)')
if ($LASTEXITCODE -ne 0 -or -not $CMakeVersionMatch.Success) {
    throw "Could not determine the CMake version from: $CMakeExecutable"
}
if ([version]$CMakeVersionMatch.Groups[1].Value -lt [version]"3.24.0") {
    throw "CMake 3.24 or newer is required; found $($CMakeVersionMatch.Groups[1].Value) at $CMakeExecutable."
}

Write-Host "Configuring native Windows build in $BuildPath"
$ConfigureArguments = @("-S", $ProjectRoot, "-B", $BuildPath, "-G", $Generator)
if ($Generator -like "Visual Studio*") { $ConfigureArguments += @("-A", $Architecture) }
& $CMakeExecutable @ConfigureArguments
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed with exit code $LASTEXITCODE." }

$CachePath = Join-Path $BuildPath "CMakeCache.txt"
$CompilerCacheEntry = Select-String -LiteralPath $CachePath -Pattern '^CMAKE_CXX_COMPILER(?::[^=]+)?=(.+)$' |
    Select-Object -First 1
if ($CompilerCacheEntry) {
    $CompilerExecutable = $CompilerCacheEntry.Matches[0].Groups[1].Value
    if ([System.IO.Path]::GetFileName($CompilerExecutable) -match '^(g\+\+|c\+\+|clang\+\+)\.exe$') {
        $CompilerDirectory = Split-Path -Parent $CompilerExecutable
        $CompilerDirectoryOnPath = @($env:PATH -split ';' |
            Where-Object { $_.TrimEnd('\') -ieq $CompilerDirectory.TrimEnd('\') }).Count -gt 0
        if ((Test-Path -LiteralPath $CompilerDirectory -PathType Container) -and
            -not $CompilerDirectoryOnPath) {
            $env:PATH = "$CompilerDirectory;$env:PATH"
        }
    }
}

Write-Host "Building chips-native ($Configuration, $Architecture)"
& $CMakeExecutable --build $BuildPath --config $Configuration --parallel $BuildParallelism
if ($LASTEXITCODE -ne 0) { throw "Native build failed with exit code $LASTEXITCODE." }

$Executable = Join-Path $BuildPath (Join-Path $Configuration "chips-native.exe")
if ($Generator -notlike "Visual Studio*" -and $Generator -ne "Ninja Multi-Config") {
    $Executable = Join-Path $BuildPath "chips-native.exe"
}
if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
    throw "Build completed but the expected executable was not found: $Executable"
}

Write-Host "Build ready: $Executable"

if ($Package) {
    $PackagePath = [System.IO.Path]::GetFullPath((Join-Path $ProjectRoot $PackageDirectory))
    if (-not $PackagePath.StartsWith($ProjectPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "PackageDirectory must resolve inside the project directory: $PackagePath"
    }
    $StagePath = Join-Path $BuildPath ("package-stage-" + [guid]::NewGuid().ToString("N"))
    if (-not $StagePath.StartsWith($ProjectPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Package staging path must resolve inside the project directory: $StagePath"
    }
    New-Item -ItemType Directory -Path $StagePath | Out-Null
    Copy-Item -LiteralPath $Executable -Destination $StagePath
    Get-ChildItem -LiteralPath (Split-Path $Executable) -Filter "*.dll" -File | Copy-Item -Destination $StagePath
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "data\CHIPS.DAT") -Destination (New-Item -ItemType Directory -Path (Join-Path $StagePath "data") -Force).FullName
    $SourceAssetManifest = Join-Path $ProjectRoot "assets\ASSET_MANIFEST.json"
    try {
        $ManifestData = Get-Content -LiteralPath $SourceAssetManifest -Raw | ConvertFrom-Json
    } catch {
        throw "Original-art asset manifest is not valid JSON: $SourceAssetManifest"
    }
    if ($ManifestData.schema_version -lt 4 -or $ManifestData.assets.Count -lt 6) {
        throw "Original-art asset manifest has an unsupported schema or incomplete asset records: $SourceAssetManifest"
    }
    $SourceAssetsRoot = [System.IO.Path]::GetFullPath((Join-Path $ProjectRoot "assets"))
    $SourceAssetsPrefix = $SourceAssetsRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar) +
        [System.IO.Path]::DirectorySeparatorChar
    $StagedAssetsRoot = Join-Path $StagePath "assets"
    New-Item -ItemType Directory -Path $StagedAssetsRoot -Force | Out-Null
    Copy-Item -LiteralPath $SourceAssetManifest -Destination $StagedAssetsRoot
    foreach ($AssetRecord in $ManifestData.assets) {
        if ([string]::IsNullOrWhiteSpace($AssetRecord.path)) {
            throw "Original-art asset manifest contains a record without a path."
        }
        $SourceAsset = [System.IO.Path]::GetFullPath((Join-Path $SourceAssetsRoot $AssetRecord.path))
        if (-not $SourceAsset.StartsWith($SourceAssetsPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Asset path escapes the source assets directory: $($AssetRecord.path)"
        }
        if (-not (Test-Path -LiteralPath $SourceAsset -PathType Leaf)) {
            throw "Manifest asset is missing from the source project: $($AssetRecord.path)"
        }
        $StagedAsset = Join-Path $StagedAssetsRoot $AssetRecord.path
        $StagedAssetDirectory = Split-Path -Parent $StagedAsset
        New-Item -ItemType Directory -Path $StagedAssetDirectory -Force | Out-Null
        Copy-Item -LiteralPath $SourceAsset -Destination $StagedAsset
    }
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "assets\ART.md") -Destination $StagedAssetsRoot
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "assets\AUDIO.md") -Destination $StagedAssetsRoot
    Copy-Item -LiteralPath (Join-Path $ProjectRoot "README.md") -Destination $StagePath
    $AssetManifest = Join-Path $StagePath "assets\ASSET_MANIFEST.json"
    if (-not (Test-Path -LiteralPath $AssetManifest -PathType Leaf)) {
        throw "Original-art asset manifest is missing from the staged package: $AssetManifest"
    }
    $StagedAssetsPath = [System.IO.Path]::GetFullPath((Join-Path $StagePath "assets"))
    $StagedAssetsPrefix = $StagedAssetsPath.TrimEnd([System.IO.Path]::DirectorySeparatorChar) +
        [System.IO.Path]::DirectorySeparatorChar
    foreach ($AssetRecord in $ManifestData.assets) {
        if ([string]::IsNullOrWhiteSpace($AssetRecord.path) -or
            [string]::IsNullOrWhiteSpace($AssetRecord.provenance)) {
            throw "Authored asset manifest contains a record without a path or provenance: $AssetManifest"
        }
        if (-not $AssetRecord.sha256) { continue }
        $StagedAsset = [System.IO.Path]::GetFullPath((Join-Path $StagedAssetsPath $AssetRecord.path))
        if (-not $StagedAsset.StartsWith($StagedAssetsPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Hashed asset path escapes the staged assets directory: $($AssetRecord.path)"
        }
        if (-not (Test-Path -LiteralPath $StagedAsset -PathType Leaf)) {
            throw "Hashed asset is missing from the staged package: $($AssetRecord.path)"
        }
        $ActualHash = (Get-FileHash -LiteralPath $StagedAsset -Algorithm SHA256).Hash
        if ($ActualHash -ne $AssetRecord.sha256) {
            throw "Asset hash mismatch for $($AssetRecord.path): expected $($AssetRecord.sha256), got $ActualHash"
        }
    }
    $SdlSourceRoot = Join-Path $BuildPath "_deps\sdl3-src"
    $SdlCachePath = Join-Path $BuildPath "CMakeCache.txt"
    if (Test-Path -LiteralPath $SdlCachePath -PathType Leaf) {
        $SdlSourceOverride = Select-String -LiteralPath $SdlCachePath `
            -Pattern '^FETCHCONTENT_SOURCE_DIR_SDL3:PATH=(.+)$' | Select-Object -First 1
        if ($SdlSourceOverride) {
            $ConfiguredSdlSource = $SdlSourceOverride.Matches[0].Groups[1].Value
            if (Test-Path -LiteralPath $ConfiguredSdlSource -PathType Container) {
                $SdlSourceRoot = $ConfiguredSdlSource
            }
        }
    }
    $SdlLicense = Join-Path $SdlSourceRoot "LICENSE.txt"
    if (-not (Test-Path -LiteralPath $SdlLicense -PathType Leaf)) {
        throw "SDL3 license was not found at $SdlLicense; cannot create a complete package."
    }
    $Notices = @"
Third-party runtime notices

SDL3
The SDL3 runtime is included with this build. SDL is licensed under the zlib license.
The license text is included below.

"@ + (Get-Content -LiteralPath $SdlLicense -Raw)
    Set-Content -LiteralPath (Join-Path $StagePath "ThirdPartyNotices.txt") -Value $Notices -Encoding utf8

    $StagedExecutable = Join-Path $StagePath "chips-native.exe"
    $StagedPack = Join-Path $StagePath "data\CHIPS.DAT"
    $SourcePack = Join-Path $ProjectRoot "data\CHIPS.DAT"
    $SourcePackHash = (Get-FileHash -LiteralPath $SourcePack -Algorithm SHA256).Hash
    $StagedPackHash = (Get-FileHash -LiteralPath $StagedPack -Algorithm SHA256).Hash
    if ($StagedPackHash -ne $SourcePackHash) {
        throw "Staged campaign data differs from the repository source: expected $SourcePackHash, got $StagedPackHash."
    }
    Write-Host "Campaign data SHA-256 (source and package match): $StagedPackHash"
    $StagedAtlas = Join-Path $StagePath "assets\art\tile-atlas.png"
    $StagedDigits = Join-Path $StagePath "assets\art\digital-digits.png"
    $StagedIcon = Join-Path $StagePath "assets\art\app-icon-polished.png"
    & $StagedExecutable --validate $StagedPack
    if ($LASTEXITCODE -ne 0) {
        throw "Staged Windows package failed its campaign-data validation (exit code $LASTEXITCODE)."
    }
    & $StagedExecutable --validate-atlas $StagedAtlas
    if ($LASTEXITCODE -ne 0) {
        throw "Staged Windows package failed its tile-atlas validation (exit code $LASTEXITCODE)."
    }
    & $StagedExecutable --validate-digits $StagedDigits
    if ($LASTEXITCODE -ne 0) {
        throw "Staged Windows package failed its scoreboard-digit validation (exit code $LASTEXITCODE)."
    }
    & $StagedExecutable --validate-icon $StagedIcon
    if ($LASTEXITCODE -ne 0) {
        throw "Staged Windows package failed its app-icon validation (exit code $LASTEXITCODE)."
    }
    & $StagedExecutable --validate-player-sprites `
        (Join-Path $StagedAssetsRoot "art\player-poses-polished.png") `
        (Join-Path $StagedAssetsRoot "art\player-swim-poses-polished.png")
    if ($LASTEXITCODE -ne 0) {
        throw "Staged Windows package failed its polished player-sprite validation (exit code $LASTEXITCODE)."
    }
    & $StagedExecutable --validate-cleaned-sprites `
        (Join-Path $StagedAssetsRoot "art\creature-sprites-polished.png") `
        (Join-Path $StagedAssetsRoot "art\key-boot-icons-polished.png")
    if ($LASTEXITCODE -ne 0) {
        throw "Staged Windows package failed its cleaned-sprite validation (exit code $LASTEXITCODE)."
    }

    New-Item -ItemType Directory -Path $PackagePath -Force | Out-Null
    $Archive = Join-Path $PackagePath "chips-native-windows-x64-$Configuration.zip"
    if (Test-Path -LiteralPath $Archive) { Remove-Item -LiteralPath $Archive -Force }
    Compress-Archive -Path (Join-Path $StagePath "*") -DestinationPath $Archive
    Remove-Item -LiteralPath $StagePath -Recurse -Force
    Write-Host "Windows x64 package ready: $Archive"
}

if ($Launch) { Start-Process -FilePath $Executable -WorkingDirectory (Split-Path $Executable) }
