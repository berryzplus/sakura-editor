# Run-Tests.ps1
Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

$productId = vswhere -property productId -version "[$VsVersion,$([int]$VsVersion + 1))"

Write-Host "`$productId is '$productId'"

$useOpenCppCoverage = ($env:GITHUB_ACTIONS -eq 'true') -or (-not ($productId -match "Enterprise$"))

# Invoke Tests1.
. "$PSScriptRoot\Run-Test.ps1" "$HomePath\$Platform\$Configuration\tests1.exe" $useOpenCppCoverage $HomePath

# install Python requirements.
$startArgs = @(
  "-m", "pip",
  "install",
  "-r", "requirements.txt"
)

# Invoke command with OpenCppCoverage.
$p = Start-Process `
  -FilePath "python.exe" `
  -ArgumentList $startArgs `
  -NoNewWindow `
  -WorkingDirectory $HomePath `
  -PassThru `
  -Wait

if ($p.ExitCode -ne 0) {
  throw "Python requirements installation was Failed."
}

# Invoke PyTest.
$startArgs = @(
  "--ignore=externals",
  "--cov",
  "--cov-config=src/test/py/.coveragerc",
  "--cov-report=xml:pytest-coverage.xml",
  "--cov-fail-under=80"
)

# Invoke command with OpenCppCoverage.
$p = Start-Process `
  -FilePath "python.exe" `
  -ArgumentList $startArgs `
  -NoNewWindow `
  -WorkingDirectory $HomePath `
  -PassThru `
  -Wait

if ($p.ExitCode -ne 0) {
  throw "PyTest was Failed."
}
