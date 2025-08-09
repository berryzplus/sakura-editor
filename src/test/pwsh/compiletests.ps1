# compiletests.ps1
Param(
  [String]$CompileTestSourceDir
)

# UTF-8エンコーディングを設定（chcp 65001相当）
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "=== CompileTests Parameters ==="
Write-Host "CompileTestSourceDir: $CompileTestSourceDir"
Write-Host "`$env:VisualStudioVersion: $env:VisualStudioVersion"
Write-Host "==============================="

Write-Host "Building project for CompileTestSourceDir: $CompileTestSourceDir"

# Visual Studio同梱のCMakeパスを取得する
$VsVersion = $($env:VisualStudioVersion -replace '^(\d+)\..+$', '$1')
$cmake = vswhere -find "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -version "[$VsVersion,$([int]$VsVersion + 1))"

# Visual Studioジェネレーターの名前を生成する
$VsProductLineVersion = vswhere -property productLineVersion -version "[$VsVersion,$([int]$VsVersion + 1))"
$generator = "Visual Studio $VsVersion $VsProductLineVersion"

# CMake configureを実行する
Write-Host "cmake --preset CompileTests -S `"$CompileTestSourceDir`""
& $cmake --preset CompileTests -S "$CompileTestSourceDir" `
  -G "$generator" -A "$env:BUILD_PLATFORM"

if ($LASTEXITCODE -ne 0) {
  throw "cmake configure was Failed."
}
