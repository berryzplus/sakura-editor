/*!	@file
	@brief プロセス生成クラス

	@author aroka
	@date 2002/01/03 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, masami shoji
	Copyright (C) 2002, aroka WinMainより分離
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcessFactory.h"

#include "_main/CControlProcess.h"
#include "_main/CNormalProcess.h"
#include "apiwrap/DarkMode.h"
#include "dlg/CDlgProfileMgr.h"

#include "CSelectLang.h"

/*!
 * @brief プロセスクラスを生成する
 * 
 * コマンドラインを解析し、妥当なプロセスクラスを生成する。
 * 
 * @param mmdLine [in] コマンドライン文字列
 * 
 * @author aroka
 * @date 2002/01/08
 * @date 2006/04/10 ryoji
 */
std::unique_ptr<CProcess> CProcessFactory::CreateInstance(std::wstring_view cmdline)
{
	// 2014.04.24 DLLの検索パスからカレントディレクトリを削除する
	::SetDllDirectoryW(L"");
	::SetSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);

	DarkMode::initDarkMode();
	DarkMode::setDarkModeConfig();
	DarkMode::setDefaultColors(true);

	// Cロケールを日本語に設定する
	::setlocale(LC_ALL, "Japanese_Japan.932");

	// 言語環境を初期化する
	CSelectLang::InitializeLanguageEnvironment();

	auto lpCmdLine = std::data(cmdline);

	//コマンドラインクラスのインスタンスを確保する
	auto pCommandLine = std::make_unique<CCommandLine>();

	SFilePath szExeFileName{ GetExeFileName().native() };
	pCommandLine->ParseKanjiCodeFromFileName(szExeFileName, szExeFileName.Length());

	pCommandLine->ParseCommandLine(lpCmdLine);

	if (!ProfileSelect(m_hInstance, lpCmdLine)) {
		return nullptr;
	}

	if( !IsValidVersion() ){
		return nullptr;
	}

	// プロセスクラスを生成する
	if (IsStartingControlProcess()) {
		return std::make_unique<CControlProcess>(m_hInstance, std::move(pCommandLine));
	} else {
		return std::make_unique<CNormalProcess>(m_hInstance, std::move(pCommandLine));
	}
}

bool CProcessFactory::ProfileSelect( HINSTANCE hInstance, LPCWSTR lpCmdLine )
{
	UNREFERENCED_PARAMETER(lpCmdLine);

	// コマンドラインオプションから起動プロファイルを判定する
	bool profileSelected = CDlgProfileMgr::TrySelectProfile( CCommandLine::getInstance() );
	if( !profileSelected ){
		CDlgProfileMgr dlgProf;
		if( dlgProf.DoModal( hInstance, nullptr, 0 ) ){
			CCommandLine::getInstance()->SetProfileName( dlgProf.m_strProfileName.c_str() );
		}else{
			return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
		}
	}
	return true;
}

/*!
	@brief Windowsバージョンのチェック
	
	Windows 95以上，Windows NT4.0以上であることを確認する．
	Windows 95系では残りリソースのチェックも行う．
	
	@author aroka
	@date 2002/01/03
*/
bool CProcessFactory::IsValidVersion()
{
	// Windowsバージョンは廃止。
	// 動作可能バージョン(=windows7以降)でなければ起動できない。
	return true;
}

/*!
	@brief コマンドラインに -NOWIN があるかを判定する。
	
	@author aroka
	@date 2002/01/03 作成 2002/01/18 変更
*/
bool CProcessFactory::IsStartingControlProcess()
{
	return CCommandLine::getInstance()->IsNoWindow();
}
