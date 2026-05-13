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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcessFactory.h"
#include "_main/CControlProcess.h"
#include "_main/CNormalProcess.h"
#include "_main/CControlTray.h"
#include "dlg/CDlgProfileMgr.h"
#include "debug/CRunningTimer.h"
#include "util/os.h"
#include <tchar.h>
#include "CSelectLang.h"
#include "config/system_constants.h"
#include "apiwrap/DarkMode.h"

class CSelectProfile final : public CProcess
{
private:
	using Base = CProcess;
	using Me = CSelectProfile;

public:
	using Base::Base;

	bool InitializeProcess(int nCmdShow [[maybe_unused]]) override {
		if (CDlgProfileMgr dlgProf; dlgProf.DoModal( G_AppInstance(), nullptr, 0)) {
			CProcess::CreateEditorProcess(std::nullopt, std::array<std::wstring, 0>(), dlgProf.m_strProfileName);
		}
		return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
	}

	bool	MainLoop() override { return false; }
};

/*!
	@brief プロセスクラスを生成する
	
	コマンドライン、コントロールプロセスの有無を判定し、
	適当なプロセスクラスを生成する。
	
	@param[in] hInstance インスタンスハンドル
	@param[in] lpCmdLine コマンドライン文字列
	
	@author aroka
	@date 2002/01/08
	@date 2006/04/10 ryoji
 */
std::unique_ptr<CProcess> CProcessFactory::CreateInstance(std::wstring_view cmdline)
{
	::setlocale(LC_ALL, "Japanese_Japan.932");

	{
		// 2014.04.24 DLLの検索パスからカレントディレクトリを削除する
		::SetDllDirectoryW( L"" );
		::SetSearchPathMode( BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT );
	}

	auto oleInit = std::make_unique<cxx::COleInit>();

	// 言語環境を初期化する
	CSelectLang::InitializeLanguageEnvironment();

	DarkMode::initDarkMode();
	DarkMode::setDarkModeConfig();
	DarkMode::setDefaultColors(true);

	auto lpCmdLine = std::data(cmdline);

	//コマンドラインクラスのインスタンスを確保する
	auto pCommandLine = std::make_unique<CCommandLine>();

	SFilePath szExeFileName(GetExeFileName().c_str());
	pCommandLine->ParseKanjiCodeFromFileName(szExeFileName, szExeFileName.Length());

	pCommandLine->ParseCommandLine(lpCmdLine);

	oleInit.reset();

	// コマンドラインオプションから起動プロファイルを判定する
	if (const auto profileSelected = CDlgProfileMgr::TrySelectProfile(CCommandLine::getInstance()); !profileSelected) {
		return std::make_unique<CSelectProfile>(m_hInstance, std::move(pCommandLine));
	}

	// プロセスクラスを生成する
	//
	// Note: 以下の処理において使用される IsExistControlProcess() は、コントロールプロセスが
	// 存在しない場合だけでなく、コントロールプロセスが起動して ::CreateMutex() を実行するまで
	// の間も false（コントロールプロセス無し）を返す。
	// 従って、複数のノーマルプロセスが同時に起動した場合などは複数のコントロールプロセスが
	// 起動されることもある。
	// しかし、そのような場合でもミューテックスを最初に確保したコントロールプロセスが唯一生き残る。
	//
	if( IsStartingControlProcess() ){
		return std::make_unique<CControlProcess>(m_hInstance, std::move(pCommandLine));
	}
	else {
		return std::make_unique<CNormalProcess>(m_hInstance, std::move(pCommandLine));
	}
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
