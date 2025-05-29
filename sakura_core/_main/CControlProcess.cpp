/*!	@file
	@brief コントロールプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcessより分離, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CControlProcess.h"
#include "CControlTray.h"
#include "env/DLLSHAREDATA.h"
#include "CCommandLine.h"
#include "env/CShareData_IO.h"
#include "debug/CRunningTimer.h"
#include "env/CShareData.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka ヘッダー整理
#include "config/system_constants.h"
#include "String_define.h"

//! HANDLE型のスマートポインタ
using HandleHolder = cxx_util::ResourceHolder<HANDLE, &CloseHandle>;

/*!
	@brief 新しいプロセスを起動する

	@return 起動したプロセスのスレッドID
 */
/* static */ bool CControlProcess::StartEditorProcess(
	_In_opt_z_ LPCWSTR pszProfileName,
	_In_opt_z_ LPCWSTR pszCurDir,
	bool sync,
	const std::vector<std::wstring>& args
)
{

	try {
		// エディタープロセスを起動する
		const auto dwThreadId = CProcess::StartSakuraProcess(pszProfileName, args, pszCurDir);

		if (sync) {
			//	起動したプロセスが完全に立ち上がるまでちょっと待つ．

			// 初期化完了イベントを作成する
			SFilePath szEventName = strprintf(L"SakuraThread-0x%08x", dwThreadId);
			HandleHolder hEvent = CreateEventW(nullptr, TRUE, FALSE, szEventName);
			if (!hEvent) {
				// L"CreateEvent()失敗。\n終了します。"
				throw basis::message_error(LS(STR_ERR_CTRLMTX2));
			}

			// 初期化完了イベントを待つ
			if (const auto waitResult = WaitForSingleObject(hEvent, 30000); WAIT_TIMEOUT == waitResult) {
				// L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"
				throw basis::message_error(LS(STR_ERR_DLGNRMPROC2));
			}
		}

		return true;
	}
	catch (const basis::message_error& e) {
		TopErrorMessage(nullptr, e.message().data());

		return false;
	}

	//if (!sync) {
	//	// タブまとめ時は起動したプロセスが立ち上がるまでしばらくタイトルバーをアクティブに保つ	// 2007.02.03 ryoji
	//	if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
	//		WaitForInputIdle( p.hProcess, 3000 );
	//		sync = true;
	//	}
	//}

	//// MYWM_FIRST_IDLE が届くまでちょっとだけ余分に待つ	// 2008.04.19 ryoji
	//// Note. 起動先プロセスが初期化処理中に COM 関数（SHGetFileInfo API なども含む）を実行すると、
	////       その時点で COM の同期機構が動いて WaitForInputIdle は終了してしまう可能性がある（らしい）。
	//if( sync && bRet )
	//{
	//	int i;
	//	for( i = 0; i < 200; i++ ){
	//		MSG msg;
	//		DWORD dwExitCode;
	//		if( ::PeekMessage( &msg, 0, MYWM_FIRST_IDLE, MYWM_FIRST_IDLE, PM_REMOVE ) ){
	//			if( msg.message == WM_QUIT ){	// 指定範囲外でも WM_QUIT は取り出される
	//				::PostQuitMessage( msg.wParam );
	//				break;
	//			}
	//			// 監視対象プロセスからのメッセージなら抜ける
	//			// そうでなければ破棄して次を取り出す
	//			if( msg.wParam == p.dwProcessId ){
	//				break;
	//			}
	//		}
	//		if( ::GetExitCodeProcess( p.hProcess, &dwExitCode ) && dwExitCode != STILL_ACTIVE ){
	//			break;	// 監視対象プロセスが終了した
	//		}
	//		::Sleep(10);
	//	}
	//}
}

//-------------------------------------------------

/*!
	@brief コントロールプロセスを初期化する
	
	MutexCPを作成・ロックする。
	CControlTrayを作成する。
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI 共有メモリを初期化するのはCProcessに移動。
	@date 2006/04/10 ryoji 初期化完了イベントの処理を追加、異常時の後始末はデストラクタに任せる
	@date 2013.03.20 novice コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
*/
bool CControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, L"CControlProcess::InitializeProcess" );

	const auto pszProfileName = CCommandLine::getInstance()->GetProfileName();

	// コントロールプロセスが起動していたら終了する
	if (CControlTray::Find(pszProfileName)) {
		return false;
	}

	SFilePath szMutexName = GSTR_MUTEX_SAKURA_CP;
	if (pszProfileName && *pszProfileName) {
		szMutexName += pszProfileName;
	}
	// プロセス初期化処理を排他制御する
	HandleHolder hMutex = CreateMutexW(nullptr, TRUE, szMutexName);
	if (!hMutex) {
		ErrorBeep();
		// L"CreateMutex()失敗。\n終了します。"
		TopErrorMessage(nullptr, LS(STR_ERR_CTRLMTX1));
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		return false;
	}

	/* 共有メモリを初期化 */
	if( !CProcess::InitializeProcess() ){
		return false;
	}

	// コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
	SFilePath szDir;
	GetSystemDirectoryW(szDir, int(std::size(szDir)));
	SetCurrentDirectoryW(szDir);

	/* 共有データのロード */
	if( !CShareData_IO::LoadShareData() ){
		/* レジストリ項目 作成 */
		CShareData_IO::SaveShareData();
	}

	/* 言語を選択する */
	CSelectLang::ChangeLang( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );
	RefreshString();

	MY_TRACETIME( cRunningTimer, L"Before new CControlTray" );

	m_pcTray = std::make_unique<CControlTray>();

	MY_TRACETIME( cRunningTimer, L"After new CControlTray" );

	hMutex = nullptr;

	/* タスクトレイにアイコン作成 */
	HWND hwnd = m_pcTray->Create( GetProcessInstance() );
	if( !hwnd ){
		ErrorBeep();
		TopErrorMessage( NULL, LS(STR_ERR_CTRLMTX3) );
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_sHandles.m_hwndTray = hwnd;

	// 初期化完了イベントをシグナル状態にする
	if (!SetSyncEvent()) {
		ErrorBeep();
		// L"SetEvent()失敗。\n終了します。"
		TopErrorMessage(nullptr, LS(STR_ERR_CTRLMTX4));
		return false;
	}

	return true;
}

/*!
	@brief コントロールプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CControlProcess::MainLoop()
{
	if( m_pcTray && GetMainWindow() ){
		m_pcTray->MessageLoop();	/* メッセージループ */
		return true;
	}
	return false;
}

/*!
	@brief コントロールプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji 共有データ保存を CControlTray へ移動
*/
void CControlProcess::OnExitProcess()
{
	GetDllShareData().m_sHandles.m_hwndTray = NULL;
}
