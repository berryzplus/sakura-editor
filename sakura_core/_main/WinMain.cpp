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
#include <Ole2.h>
#include <locale.h>
#include "_main/CCommandLine.h"
#include "CProcessFactory.h"
#include "CProcess.h"
#include "util/ole/OleInit.hpp"
#include "util/os.h"
#include "util/MessageBoxF.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"
#include "version.h"
#include "util/std_macro.h"
#include "env/DLLSHAREDATA.h"
#include "apiwrap/DarkMode.h"

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
	_In_ HINSTANCE		hInstance,		//!< handle to current instance
	_In_opt_ HINSTANCE	hPrevInstance [[maybe_unused]],	//!< handle to previous instance
	_In_ LPWSTR			lpCmdLine,		//!< pointer to command line
	_In_ int			nCmdShow		//!< show state of window
)
{
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	// 2009.9.10 syat メモリリークチェックを追加
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	MY_RUNNINGTIMER(cRunningTimer, L"WinMain" );
	{
		// 2014.04.24 DLLの検索パスからカレントディレクトリを削除する
		::SetDllDirectory( L"" );
		::SetSearchPathMode( BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT );

		setlocale( LC_ALL, "Japanese" ); //2007.08.16 kobake 追加
	}

	// COMライブラリを初期化する
	auto oleInit = std::make_unique<cxx::OleInit>();
	
	//開発情報
	DEBUG_TRACE(L"-- -- WinMain -- --\n");
	DEBUG_TRACE(L"sizeof(DLLSHAREDATA) = %d\n",sizeof(DLLSHAREDATA));

	DarkMode::initDarkMode();
	DarkMode::setDarkModeConfig();
	DarkMode::setDefaultColors(true);

	//コマンドラインクラスのインスタンスを確保する
	CCommandLine cCommandLine;

	try{
		//プロセスの生成とメッセージループ
		CProcessFactory aFactory;
		auto process = aFactory.Create( hInstance, lpCmdLine );
		MY_TRACETIME( cRunningTimer, L"ProcessObject Created" );

		if (!process) return 1;

		(void)nCmdShow;	//TODO: Runに渡す

		process->Run();
		delete process;

		return 0;	//TODO: Runの戻り値を返すように変える
	}
	// メモリ不足エラーが発生した場合
	catch (const std::bad_alloc&) {
		ErrorBeep();
		TopErrorMessage(nullptr, L"メモリ不足です。\n終了します。");	//TODO: リソースに入れる
		return 4;	// 仮の終了コード
	}
	// その他エラーが発生した場合
	catch (const std::exception&) {
		ErrorBeep();
		TopErrorMessage(nullptr, L"不明なエラーが発生しました。\n終了します。");	//TODO: リソースに入れる
		return 5;	// 仮の終了コード
	}
}
