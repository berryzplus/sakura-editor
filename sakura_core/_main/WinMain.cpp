/*!	@file
	@brief Entry Point

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date	2001/06/26 genta ワード単位のGrepのためのコマンドライン処理追加
	@date	2002/01/08 aroka 処理の流れを整理、未使用コードを削除
	@date	2002/01/18 aroka 虫取り＆リリース
	@date	2009/01/07 ryoji WinMainにOleInitialize/OleUninitializeを追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, aroka
	Copyright (C) 2007, kobake
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcessFactory.h"

#include "env/DLLSHAREDATA.h"

/*!
	Windows Entry point

	コマンドラインオプションで -NOWIN を指定するかどうかでプロセスのタイプが変わる。
		+----------+---------------------------+---------------------------+
		|-NOWIN指定|どうなる？                 |作成するProcessインスタンス|
		+----------+---------------------------+---------------------------+
		|有        |コントロールプロセスとなる |CControlProcessクラス      |
		+----------+---------------------------+---------------------------+
		|無        |エディタプロセスとなる     |CNormalProcessクラス       |
		+----------+---------------------------+---------------------------+
*/
int WINAPI wWinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	[[maybe_unused]] HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPWSTR		lpCmdLine,		//!< pointer to command line
	[[maybe_unused]] int			nCmdShow		//!< show state of window
)
{
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	// 2009.9.10 syat メモリリークチェックを追加
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	MY_RUNNINGTIMER(cRunningTimer, L"WinMain" );

	//開発情報
	DEBUG_TRACE(L"-- -- WinMain -- --\n");
	DEBUG_TRACE(L"sizeof(DLLSHAREDATA) = %d\n",sizeof(DLLSHAREDATA));

	cxx::COleInit oleInit;
	if (!oleInit) return 1;	// 暫定エラーコード=1は「assert failedと同じ」。

	(void)oleInit;	// OLEのクリーンアップはスコープを抜けるときに行わせる

	//プロセスの生成とメッセージループ
	try {
		const auto process = CProcessFactory(hInstance).CreateInstance(lpCmdLine);
		MY_TRACETIME( cRunningTimer, L"ProcessObject Created" );

		if (process) {
			process->Run();
		}
	}
	catch (const std::domain_error& e) {
		ErrorBeep();
		TopErrorMessage(nullptr, cxx::to_wstring(e.what()).c_str());
		return 3;	// 暫定エラーコード=3は「terminatedと同じ」。
	}

	return 0;
}
