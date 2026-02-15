# Setup script for libwebsockets on Windows
# This script downloads pre-built libwebsockets or builds from source

param(
    [switch]$Force = $false
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$PluginDir = Split-Path -Parent $ScriptDir
$ThirdPartyDir = Join-Path $PluginDir "ThirdParty\libwebsockets"
$BuildDir = Join-Path $env:TEMP "libwebsockets-build-$(Get-Random)"

$LibWebSocketsVersion = "v4.3.2"
$LibWebSocketsRepo = "https://github.com/warmcat/libwebsockets.git"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "libwebsockets Auto-Setup Script (Windows)" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Plugin Directory: $PluginDir"
Write-Host "ThirdParty Directory: $ThirdPartyDir"
Write-Host "Build Directory: $BuildDir"
Write-Host ""

# Check if already set up
$HeaderPath = Join-Path $ThirdPartyDir "include\libwebsockets.h"
$LibPath = Join-Path $ThirdParty Dir "lib\Win64\websockets_static.lib"

if ((Test-Path $HeaderPath) -and (Test-Path $LibPath) -and -not $Force) {
    Write-Host "✓ libwebsockets already set up" -ForegroundColor Green
    Write-Host "  Headers: $HeaderPath" -ForegroundColor Green
    Write-Host "  Library: $LibPath" -ForegroundColor Green
    Write-Host ""
    Write-Host "Use -Force to rebuild" -ForegroundColor Yellow
    exit 0
}

Write-Host "⚠ libwebsockets not found or incomplete, building from source..." -ForegroundColor Yellow
Write-Host ""

# Check for required tools
try {
    $null = Get-Command git -ErrorAction Stop
} catch {
    Write-Error "git is required but not installed. Please install Git for Windows."
    exit 1
}

try {
    $null = Get-Command cmake -ErrorAction Stop
} catch {
    Write-Error "cmake is required but not installed. Please install CMake."
    exit 1
}

# Create build directory
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
Set-Location $BuildDir

# Clone libwebsockets
Write-Host "→ Cloning libwebsockets $LibWebSocketsVersion..." -ForegroundColor Yellow
git clone --depth 1 --branch $LibWebSocketsVersion $LibWebSocketsRepo libwebsockets
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to clone libwebsockets"
    exit 1
}

Set-Location (Join-Path $BuildDir "libwebsockets")

# Create build directory
New-Item -ItemType Directory -Force -Path "build" | Out-Null
Set-Location "build"

# Configure CMake
Write-Host "→ Configuring build with CMake..." -ForegroundColor Yellow
cmake .. `
    -DCMAKE_BUILD_TYPE=Release `
    -DLWS_WITH_SHARED=OFF `
    -DLWS_WITH_STATIC=ON `
    -DLWS_WITH_SSL=ON `
    -DLWS_WITH_ZLIB=ON `
    -DLWS_IPV6=ON `
    -DLWS_WITHOUT_TESTAPPS=ON `
    -DLWS_WITHOUT_TEST_SERVER=ON `
    -DLWS_WITHOUT_TEST_CLIENT=ON `
    -A x64

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed"
    exit 1
}

# Build
Write-Host "→ Building libwebsockets..." -ForegroundColor Yellow
cmake --build . --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

# Create target directories
Write-Host "→ Creating target directories..." -ForegroundColor Yellow
$IncludeDir = Join-Path $ThirdPartyDir "include"
$LibDir = Join-Path $ThirdPartyDir "lib\Win64"
New-Item -ItemType Directory -Force -Path $IncludeDir | Out-Null
New-Item -ItemType Directory -Force -Path $LibDir | Out-Null

# Copy headers
Write-Host "→ Copying headers..." -ForegroundColor Yellow
$SourceIncludeDir = Join-Path (Split-Path -Parent (Get-Location)) "include"
Copy-Item -Path "$SourceIncludeDir\*.h" -Destination $IncludeDir -Force

# Copy library
Write-Host "→ Copying library..." -ForegroundColor Yellow
$ReleaseLibPath = Join-Path (Get-Location) "lib\Release\websockets_static.lib"
if (Test-Path $ReleaseLibPath) {
    Copy-Item -Path $ReleaseLibPath -Destination $LibDir -Force
} else {
    # Try alternate paths
    $AltPath = Join-Path (Get-Location) "Release\websockets_static.lib"
    if (Test-Path $AltPath) {
        Copy-Item -Path $AltPath -Destination $LibDir -Force
    } else {
        Write-Warning "Could not find websockets_static.lib in expected locations"
        Get-ChildItem -Path (Get-Location) -Recurse -Filter "*.lib" | ForEach-Object {
            Write-Host "  Found: $($_.FullName)"
        }
    }
}

# Clean up
Write-Host "→ Cleaning up build directory..." -ForegroundColor Yellow
Set-Location $env:TEMP
Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "=========================================" -ForegroundColor Green
Write-Host "✓ libwebsockets setup complete!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green
Write-Host "Headers: $IncludeDir"
Write-Host "Library: $LibDir"
Write-Host ""

exit 0
