#Run-Test.ps1
Param(
    [String]$useSonarQube = "",
    [String]$coverageOutPath,
    [String]$testCommand,
    [String]$testCommandArgs = "`r`n"
)

if ($useSonarQube -eq "Yes") {
    . "$PSScriptRoot\Run-OpenCppCoverage.ps1" `
        $coverageOutPath `
        $testCommand `
        $testCommandArgs

    return 0
}

# ホームパスを取得する
$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..")

$p = Start-Process `
    -FilePath $testCommand `
    -ArgumentList $($testCommandArgs -split "`r`n" | Where-Object { $_ -ne '' }) `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
    throw "$(Split-Path -Path $testCommand -Leaf) was Failed."
}

