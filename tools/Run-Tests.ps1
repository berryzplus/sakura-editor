#Run-Tests.ps1
Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$useSonarQube = ""
)

# ホームパスを取得する
$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")

# Invoke Tests1.
. "$PSScriptRoot\Run-Test.ps1" `
    $useSonarQube `
    "tests1-coverage.xml" `
    "$HomePath\$Platform\$Configuration\tests1.exe" `
    "--gtest_output=xml:tests1-googletest.xml"
