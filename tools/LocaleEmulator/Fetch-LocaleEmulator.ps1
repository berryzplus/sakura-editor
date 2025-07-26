# Fetch-LocaleEmulator.ps1
Param(
    [String]$LEExpandDir = $PSScriptRoot
)

# LocaleEmulatorが存在しなければダウンロードする
if (-not(Test-Path "$LEExpandDir\LEProc.exe")) {
    # LocaleEmulatorをダウンロードする
    if (-not(Test-Path "$LEExpandDir\locale-emulator.zip")) {
        Invoke-WebRequest -OutFile "$LEExpandDir\locale-emulator.zip" "https://github.com/xupefei/Locale-Emulator/releases/download/v2.5.0.1/Locale.Emulator.2.5.0.1.zip" `
    }

    # LocaleEmulatorを展開する
    if (-not(Test-Path "$LEExpandDir\LEProc.exe")) {
        # zipを展開する
        7z x "$LEExpandDir\locale-emulator.zip" "-o$LEExpandDir" "*"
    }

    # AutoHotKeyをインストールする
    cmd.exe /C "where.exe AutoHotKey.exe >NUL"
    if ($LASTEXITCODE -ne 0) {
        choco install autohotkey.install -y
    }

    # AutoHotKeyを起動する
    Start-Process "AutoHotKey" "$LEExpandDir\init-locale-emulator.ahk"

    # LocaleEmulatorをインストールする
    Start-Process "$LEExpandDir\LEInstaller.exe"
}
