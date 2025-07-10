Param(
    [String]$Platform = $env:BUILD_PLATFORM,
    [String]$Configuration = $env:BUILD_CONFIGURATION,
    [String]$VsVersion = $($(vswhere -latest -property catalog_productDisplayVersion) -replace '^(\d+)\..+$', '$1'),
    [String]$HomePath = $PSScriptRoot
)

$CMD_MSBUILD = $(vswhere -find 'MSBuild\**\Bin\MSBuild.exe' -version "[$VsVersion,$([int]$VsVersion + 1)`)")

$p = Start-Process `
    -FilePath $CMD_MSBUILD `
    -ArgumentList @("/p:Platform=$Platform", "/p:Configuration=$Configuration", "/t:Build", "/flp:logfile=msbuild-$Platform-$Configuration.log") `
    -NoNewWindow `
    -WorkingDirectory $HomePath `
    -PassThru `
    -Wait

if ($p.ExitCode -ne 0) {
  throw "MsBuild was Failed."
}
