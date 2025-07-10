# Run-SonarScanner.ps1
Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

$SonarScannerProperties = "$HomePath\.sonar\scanner\conf\sonar-scanner.properties"

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.organization=.+").Matches.Success)) {
    Throw "Missing 'sonar.organization' in $SonarScannerProperties."
}

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.projectKey=.+").Matches.Success)) {
    Throw "Missing 'sonar.projectKey' in $SonarScannerProperties."
}

if (-not((Get-Content $SonarScannerProperties | Select-String "^sonar.host.url=.+").Matches.Success)) {
    Throw "Missing 'sonar.host.url' in $SonarScannerProperties."
}

# Fetch the sonar-scanner.
. "$PSScriptRoot\Fetch-SonarScanner.ps1"

# SONAR_TOKEN未定義の場合、ファイルから取得を試みる
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    $env:SONAR_TOKEN = "$(Get-Content $HomePath\SONAR_TOKEN)"
}

# それでもSONAR_TOKEN未定義の場合、異常終了する
if ([string]::IsNullOrEmpty($env:SONAR_TOKEN)) {
    Throw "`$env:SONAR_TOKEN is not defined"
}

# Run SonarScanner.
$p = Start-Process `
    -FilePath $HomePath\.sonar\scanner\bin\sonar-scanner.bat `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
    throw "SonarScanner was Failed."
}

# SonarSourceはsonarscanner-cliのDockerイメージも提供している。
# これを使うとローカル環境に依存せずSonarScannerを実行できるはずだがC/C++には未対応らしい。

# docker run `
#     --rm `
#     -e SONAR_HOST_URL="$env:SONAR_HOST_URL"  `
#     -e SONAR_TOKEN="$env:SONAR_TOKEN" `
#     -v "$($HomePath):/usr/src" `
#     sonarsource/sonar-scanner-cli `
#     -D"sonar.organization=berryzplus" `
#     -D"sonar.projectKey=berryzplus_sakura-editor" `
#     -D"sonar.branch.name=work"
# 
# if ($LASTEXITCODE -ne 0) {
#     throw "SonarScanner was Failed."
# }
