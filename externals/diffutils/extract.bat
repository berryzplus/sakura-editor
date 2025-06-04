@echo off

where.exe 7z.exe >NUL 2>&1
if errorlevel 1 (
    echo 7z.exe が見つかりません。
    exit /b 1
)

REM diffutils-2.8.7-1-bin.zip から bin/diff.exe を抽出するバッチ

set ZIPFILE=diffutils-2.8.7-1-bin.zip
set EXTRACTFILE=bin/diff.exe

if not exist "%ZIPFILE%" (
    echo %ZIPFILE% が見つかりません。
    exit /b 1
)

7z.exe e "%ZIPFILE%" "%EXTRACTFILE%" -aoa
if errorlevel 1 (
    echo 抽出に失敗しました。
    exit /b 1
)

REM diffutils-2.8.7-1-dep.zip から bin/*.dll を抽出
set DEPZIP=diffutils-2.8.7-1-dep.zip
set EXTRACTDLL=bin\*.dll

if not exist "%DEPZIP%" (
    echo %DEPZIP% が見つかりません。
    exit /b 1
)

7z.exe e "%DEPZIP%" "%EXTRACTDLL%" -aoa
if errorlevel 1 (
    echo DLL抽出に失敗しました。
    exit /b 1
)

echo 抽出が完了しました。
