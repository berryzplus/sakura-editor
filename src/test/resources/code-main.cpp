/*! @file */
/*
	Copyright (C) 2018-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "pch.h"
#include "testing/StartEditorProcess.hpp"

/*!
 * テストモジュールのエントリポイント
 */
int wmain(int argc, wchar_t **argv) {

	// ロケールID(日本語、日本)を導出する
	const auto langId = MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
	const auto lcid = MAKELCID(langId, SORT_DEFAULT);

	// コマンドラインに -PROF 指定がある場合、wWinMainを起動して終了する。
	if (std::wstring command(GetCommandLineW());
		std::regex_search(command, std::wregex(LR"(-PROF\b)", std::wregex::icase))) {
		// コマンドライン文字列の先頭に入っているアプリパスを除去する
		if (std::wsmatch m; std::regex_match(command, m, std::wregex(LR"(^(?:".+?"|\S+)\s+(.+))"))) {
			command = m[1];
		}

		// テスト実行時のロケールは日本語に固定する
		SetThreadUILanguage(lcid);	// スレッドのUI言語を変更

		// wWinMainを起動して結果を返して抜ける
		return testing::StartEditorProcess(command);
	}

	// WinMainを起動しない場合、標準のgmock_main同様の処理を実行する。
	// InitGoogleMock は Google Test の初期化も行うため、InitGoogleTest を別に呼ぶ必要はない。
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleMock(&argc, argv);

	// テスト実行時のロケールは日本語に固定する
	SetThreadUILanguage(lcid);	// スレッドのUI言語を変更

	// Cロケールを日本語に固定
	std::setlocale(LC_ALL, "Japanese_Japan.932");

	return RUN_ALL_TESTS();
}
