# cmaketools.ps1
Param(
  [String]$Platform = "x64",
  [String]$Configuration = "Debug",
  [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
  [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..\.."),
  [String]$VcpkgInstalledDir = "$HomePath\build\$(Platform)\vcpkg_installed",
  [String]$CMakeToolsBuildDir = "$HomePath\build\$Platform\CMakeTools\",
  [String]$CMakeToolsOutDir = "$HomePath\$Platform\"
)

# UTF-8エンコーディングを設定（chcp 65001相当）
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "=== CMake Tools Parameters ==="
Write-Host "Platform: $Platform"
Write-Host "Configuration: $Configuration"
Write-Host "VsVersion: $VsVersion"
Write-Host "HomePath: $HomePath"
Write-Host "VcpkgInstalledDir: $VcpkgInstalledDir"
Write-Host "CMakeToolsBuildDir: $CMakeToolsBuildDir"
Write-Host "CMakeToolsOutDir: $CMakeToolsOutDir"
Write-Host "==============================="

Write-Host "Building project for Platform: $Platform, Configuration: $Configuration, VsVersion: $VsVersion"

# ターゲットトリプレットを導出する
$triplet = "$Platform-windows-static"

# プラットフォーム指定をCPUアーキテクチャーに変換する
if ("x64" -eq $Platform) {
  $arch = "amd64"
} elseif ("ARM64" -eq $Platform) {
  $arch = "arm64"
  $triplet = "$arch-windows-static"
} elseif ("Win32" -eq $Platform) {
  $arch = "x86"
  $triplet = "$arch-windows-static"
} else {
  throw "Unsupported Platform: $Platform"
}

# Visual Studio同梱のCMakeパスを取得する
$VsInstallDir = vswhere -property installationPath -version "[$VsVersion,$([int]$VsVersion + 1))"
$cmake = "$VsInstallDir\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

# Visual Studioジェネレーターの名前を生成する
$VsProductLineVersion = vswhere -property productLineVersion -version "[$VsVersion,$([int]$VsVersion + 1))"
$generator = "Visual Studio $VsVersion $VsProductLineVersion"

# ビルドディレクトリが存在しない場合は作成する
if (-not(Test-Path "$CMakeToolsBuildDir")) {
  New-Item -Path "$CMakeToolsBuildDir" -ItemType Directory -Force | Out-Null
}

# CMake configureを実行する
Write-Host "cmake -G `"$generator`" -A `"$Platform`" -B `"$CMakeToolsBuildDir`" -S `"$HomePath`""
& $cmake -G "$generator" -A "$Platform" -B "$CMakeToolsBuildDir" -S "$HomePath" `
  -DCMAKE_CONFIGURATION_TYPES="Debug;Release" `
  -DVCPKG_HOST_TRIPLET="x64-windows" `
  -DVCPKG_TARGET_TRIPLET="$triplet" `
  -DCMAKE_TOOLCHAIN_FILE:FILEPATH="$HomePath\tools\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DVCPKG_INSTALLED_DIR:FILEPATH="$VcpkgInstalledDir" `
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:FILEPATH="$CMakeToolsOutDir"

if ($LASTEXITCODE -ne 0) {
  throw "cmake configure was Failed."
}
