# Run-Tests.ps1
Param(
    [String]$Platform = "x64",
    [String]$Configuration = "Debug",
    [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
    [String]$HomePath = [System.IO.Path]::GetFullPath("$PSScriptRoot\..\..")
)

# Invoke Tests1.
. "$PSScriptRoot\Run-Test.ps1" "$HomePath\$Platform\$Configuration\tests1.exe" $VsVersion $HomePath
