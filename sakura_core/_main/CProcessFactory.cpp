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

#include "_main/CCommandLine.h"
#include "dlg/CDlgProfileMgr.h"

/*!
	@brief プロセスクラスを生成する
	
	コマンドライン、コントロールプロセスの有無を判定し、
	適当なプロセスクラスを生成する。
	
	@param[in] lpCmdLine コマンドライン文字列
	
	@author aroka
	@date 2002/01/08
	@date 2006/04/10 ryoji
 */
std::unique_ptr<CProcess> CProcessFactory::CreateInstance(_In_z_ LPCWSTR lpCmdLine) noexcept
{
	//コマンドラインクラスのインスタンスを確保する
	CCommandLine cCommandLine;

	// 言語環境を初期化する
	CSelectLang::InitializeLanguageEnvironment();

	// 実行ファイル名を取得する
	SFilePath szExePath = GetExeFileName();

	// 実行ファイル名をもとに漢字コードを固定する．
	cCommandLine.ParseKanjiCodeFromFileName(szExePath, int(std::size(szExePath)));

	// コマンドラインを解析する
	cCommandLine.ParseCommandLine(lpCmdLine);

	if (!ProfileSelect()) {
		CSelectLang::UninitializeLanguageEnvironment();
		return nullptr;
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
	if (CCommandLine::getInstance()->IsNoWindow()) {
		return std::make_unique<CControlProcess>(m_hInstance, std::move(cCommandLine));
	}
	else{
		return std::make_unique<CNormalProcess>(m_hInstance, std::move(cCommandLine));
	}
}

bool CProcessFactory::ProfileSelect() noexcept
{
	auto& cCommandLine = *CCommandLine::getInstance();

	// プロファイル選択済みならすぐ抜ける
	if (const auto profileSelected = CDlgProfileMgr::TrySelectProfile(&cCommandLine)) {
		return true;
	}

	// プロファイルマネージャを表示する
	CDlgProfileMgr dlgProf;
	if (!dlgProf.DoModal(m_hInstance, nullptr, 0)) {
		return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
	}

	// プロファイル名が変更されてなければそのまま抜ける
	if (const auto pszProfileName = cCommandLine.GetProfileName(); !pszProfileName || dlgProf.m_strProfileName == pszProfileName) {
		return true;
	}

	cCommandLine.SetProfileName(dlgProf.m_strProfileName.c_str());
	return true;
}


