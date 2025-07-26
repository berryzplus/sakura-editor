# Get-BuildId.ps1
Param(
    [String]$commitHash # ${{ github.sha }}
)

if ([String]::IsNullOrEmpty($env:GITHUB_RUN_NUMBER)) {
    # ローカルビルドは固定で buildLocal を入れる
    $build_id += "-buildLocal"

} else {
    # プロジェクト以外のリポジトリではアカウント名を入れる
    if ('sakura-editor/sakura' -ne $env:GITHUB_REPOSITORY) {
        $build_id += "-$env:GITHUB_ACTOR"
    }

    # プルリクエストならプルリクエスト番号を入れる
    if ('pull-request' -eq "$env:GITHUB_EVENT_NAME" -and "$env:GITHUB_REF_NAME" -match "^(\d+)/merge$") {
        $build_id += "-PR$('{0:D5}' -f [int]$matches[1])"
    }
    # タグビルド（リリース版）ならタグ名を入れる
    elseif (-not([String]::IsNullOrEmpty($env:GITHUB_TAG_NAME))) {
        $build_id += "-$env:GITHUB_TAG_NAME"
    }

    # ビルド番号（ワークフローの通し番号）を入れる
    $build_id += "-build$('{0:D5}' -f [int]$env:GITHUB_RUN_NUMBER)"
}

# コミットハッシュの先頭8桁を入れる
$build_id += "-$($commitHash.SubString(0, 8))"

$build_id
