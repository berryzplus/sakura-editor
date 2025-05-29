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
#include "CProcessFactory.h"
#include "CControlProcess.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CControlTray.h"
#include "dlg/CDlgProfileMgr.h"
#include "debug/CRunningTimer.h"
#include "util/os.h"
#include <io.h>
#include <tchar.h>
#include "CSelectLang.h"
#include "config/system_constants.h"

class CProcess;

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
CProcess* CProcessFactory::Create( HINSTANCE hInstance, LPCWSTR lpCmdLine )
{
	// 言語環境を初期化する
	CSelectLang::InitializeLanguageEnvironment();

	if( !ProfileSelect( hInstance, lpCmdLine ) ){
		CSelectLang::UninitializeLanguageEnvironment();
		return nullptr;
	}

	CProcess* process = nullptr;
	if( !IsValidVersion() ){
		return 0;
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
			process = new CControlProcess( hInstance, lpCmdLine );
	}
	else{
			process = new CNormalProcess( hInstance, lpCmdLine );
	}
	return process;
}

bool CProcessFactory::ProfileSelect( HINSTANCE hInstance, LPCWSTR lpCmdLine )
{
	//	May 30, 2000 genta
	//	実行ファイル名をもとに漢字コードを固定する．
	WCHAR szExeFileName[MAX_PATH];
	const int cchExeFileName = ::GetModuleFileName(NULL, szExeFileName, _countof(szExeFileName));
	CCommandLine::getInstance()->ParseKanjiCodeFromFileName(szExeFileName, cchExeFileName);

	CCommandLine::getInstance()->ParseCommandLine(lpCmdLine);

	// コマンドラインオプションから起動プロファイルを判定する
	bool profileSelected = CDlgProfileMgr::TrySelectProfile( CCommandLine::getInstance() );
	if( !profileSelected ){
		CDlgProfileMgr dlgProf;
		if( dlgProf.DoModal( hInstance, NULL, 0 ) ){
			CCommandLine::getInstance()->SetProfileName( dlgProf.m_strProfileName.c_str() );
		}else{
			return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
		}
	}
	return true;
}


