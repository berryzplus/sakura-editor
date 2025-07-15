# ビルドに使用するバッチファイル

<!-- TOC -->

- [ビルドに使用するバッチファイル](#ビルドに使用するバッチファイル)
  - [使用するバッチファイルの一覧](#使用するバッチファイルの一覧)
    - [関連情報](#関連情報)
  - [呼び出し構造](#呼び出し構造)
  - [ビルドに使用するバッチファイルの引数](#ビルドに使用するバッチファイルの引数)
  - [バッチファイルの仕組み](#バッチファイルの仕組み)
    - [githash.bat の構造](#githashbat-の構造)
      - [処理の流れ](#処理の流れ)
    - [postBuild.bat の構造](#postbuildbat-の構造)
      - [処理の流れ](#処理の流れ-1)

<!-- /TOC -->

## 使用するバッチファイルの一覧

| ファイル名 | 説明 |
----|---- 
|[build-all.bat](../build-all.bat)| すべてをビルドできるバッチファイル  |
|[build-sln.bat](../build-sln.bat) | solution をビルドする |
|[build-gnu.bat](../build-gnu.bat) | Makefile をビルドする |
|[sakura\preBuild.bat](../sakura/preBuild.bat) | 特に何もしない |
|[sakura\githash.bat](../sakura/githash.bat) | Git や CI の環境変数から githash.h を生成する |
|[sakura\postBuild.bat](../sakura/postBuild.bat) | bregonig.dll と ctags.exe を展開しコピーする |
|[tests\googletest.build.cmd](../tests/googletest.build.cmd) | Google Test をビルドする |
|[tests\compiletests.run.cmd](../tests/compiletests.run.cmd) | コンパイルテストを実行する |
|[build-chm.bat](../build-chm.bat) | compiled HTML ファイルをビルドする |
|[build-installer.bat](../build-installer.bat) | インストーラをビルドする |

### 関連情報

SonarQube に関しては [こちら](../tools/SonarQube/SonarQube.adoc) も参照してください。

## 呼び出し構造

- [build-all.bat](../build-all.bat)
    - [build-sln.bat](../build-sln.bat)
        - MSBuild.exe sakura.sln
            - [sakura\preBuild.bat](../sakura/preBuild.bat)
            - HeaderMake.exe : Funccode_define.h, Funccode_enum.h を生成する
            - [sakura\githash.bat](../sakura/githash.bat)
                - git.exe
            - [sakura\postBuild.bat](../sakura/postBuild.bat)
                - [tools\zip\unzip.bat](../tools/zip/unzip.bat)
                    - 7z.exe または [tools\zip\unzip.ps1](../tools/zip/unzip.ps1)
            - [tests\googletest.build.cmd](../tests/googletest.build.cmd)
                - git.exe
                - cmake.exe
            - [tests\compiletests.run.cmd](../tests/compiletests.run.cmd)
                - cmake.exe
    - [build-gnu.bat](../build-gnu.bat)
        - mingw32-make.exe sakura_core
            - [sakura\githash.bat](../sakura/githash.bat)
                - git.exe
            - HeaderMake.exe
    - [build-chm.bat](../build-chm.bat)
        - [help\remove-comment.py](../help/remove-comment.py) : [sakura_core\sakura.hh](../sakura_core/sakura.hh) に記述された日本語を含む行コメントを削除する
        - ChmSourceConverter.exe : ヘルプファイルの文字コードを UTF-8 から Shift_JIS に変換する
        - [help\CompileChm.ps1](../help/CompileChm.ps1)
            - hhc.exe (Visual Studio に同梱) : compiled HTML をビルドするコンパイラ。かなり古いツールであり、日本語 HTML をビルドするためには Windows のシステムロケールを日本語に変更する必要がある。
    - [build-installer.bat](../build-installer.bat)
        - ISCC.exe : [InnoSetup](https://www.jrsoftware.org/isinfo.php) でインストーラをビルドする

## ビルドに使用するバッチファイルの引数

| バッチファイル | 第一引数 | 第二引数 |
----|----|----
|build-all.bat       | platform ("Win32" または "x64" または "MinGW") | configuration ("Debug" または "Release")  |
|build-sln.bat       | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|build-gnu.bat       | platform ("MinGW") | configuration ("Debug" または "Release")  |
|sakura\preBuild.bat | HeaderMake.exe の実行ファイルのフォルダーパス | なし |
|sakura\postBuild.bat| platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |
|build-chm.bat       | なし | なし |
|build-installer.bat | platform ("Win32" または "x64") | configuration ("Debug" または "Release")  |

## バッチファイルの仕組み

### githash.bat の構造

#### 処理の流れ

- Git や CI の環境変数を元に githash.h を生成する
    - 設定される環境変数については [こちら](build-envvars.md) を参照してください。

### postBuild.bat の構造

#### 処理の流れ

* リポジトリに登録している bregonig と ctags の zip ファイルを解凍して bregonig.dll と ctags.exe を sakura.exe のビルド出力先にコピーする
