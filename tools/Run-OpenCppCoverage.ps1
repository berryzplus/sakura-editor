# Run-OpenCppCoverage.ps1
Param(
    [String]$coverageOutPath,
    [String]$testCommand,
    [String]$testCommandArgs = "`r`n"
)

# ホームパスを取得する
$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")

# テストコマンドをフルパスにする
$testCommand = [System.IO.Path]::GetFullPath($testCommand)

# OpenCppCoverageの引数配列を作成する
$openCppCoverageArgs = @(
  "--export_type xml:$HomePath\$coverageOutPath",
  "--modules $testCommand",
  "--sources $HomePath",
  "--excluded_sources $HomePath\build",
  "--working_dir $([System.IO.Path]::GetDirectoryName($testCommand))",
  "--cover_children",
  "--",
  $testCommand
)

# testCommandArgsを改行で分割して配列化し空要素を省いてopenCppCoverageの引数配列に追加する
$openCppCoverageArgs += $testCommandArgs -split "`r`n" | Where-Object { $_ -ne '' }

# Invoke command with OpenCppCoverage.
$p = Start-Process `
    -FilePath "C:\Program Files\OpenCppCoverage\OpenCppCoverage.exe" `
    -ArgumentList $openCppCoverageArgs `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "$(Split-Path -Path $testCommand -Leaf) was Failed."
}
