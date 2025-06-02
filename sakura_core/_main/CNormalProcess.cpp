/*!	@file
	@brief エディタプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CProcessより分離
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, genta, Moca, MIK
	Copyright (C) 2004, Moca, naoh
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, Uchi
	Copyright (C) 2009, syat, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CControlTray.h"
#include "window/CEditWnd.h" // 2002/2/3 aroka
#include "CGrepAgent.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h" // 2003/03/28 MIK
#include "debug/CRunningTimer.h"
#include "util/window.h"
#include "util/file.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "CAppMode.h"
#include "env/CDocTypeManager.h"
#include "apiwrap/StdApi.h"
#include "CSelectLang.h"
#include "env/CShareData.h"
#include "config/system_constants.h"

#include "String_define.h"

//! HANDLE型のスマートポインタ
using HandleHolder = cxx_util::ResourceHolder<HANDLE, &CloseHandle>;

/*!
 * @brief コントロールプロセスを起動する
 *
 * 既存CProcessFactory::StartControlProcess()と概ね等価です。
 */
/* static */ bool CNormalProcess::StartControlProcess(_In_opt_z_ LPCWSTR pszProfileName)
{
	try {
		// コントロールプロセスを起動する
		const auto dwThreadId = StartSakuraProcess(pszProfileName, { L"-NOWIN" });

		// 初期化完了イベントを作成する
		SFilePath szEventName = strprintf(L"SakuraThread-0x%08x", dwThreadId);
		HandleHolder hEvent = CreateEventW(nullptr, TRUE, FALSE, szEventName);
		if (!hEvent) {
			// L"CreateEvent()失敗。\n終了します。"
			throw basis::message_error(LS(STR_ERR_CTRLMTX2));
		}

		// トレイウインドウが作成されるのを待つ
		HWND hWndTray = nullptr;
		const auto startTick = GetTickCount64();
		while (!hWndTray && GetTickCount64() - startTick < 5 * 1000) {
			
			hWndTray = CControlTray::Find(pszProfileName);
			if (!hWndTray) {
				//do nothing
			}
			else if (IsWindowEnabled(hWndTray)) {
				break;
			}
			else {
				hWndTray = nullptr;
			}
	
			Sleep(10);  // 10msスリープしてリトライ
		}

		if (!hWndTray) {
			// L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"
			throw basis::message_error(LS(STR_ERR_DLGNRMPROC2));
		}

		// 初期化完了イベントを待つ
		if (const auto waitResult = WaitForSingleObject(hEvent, 30000); WAIT_TIMEOUT == waitResult) {
			// L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"
			throw basis::message_error(LS(STR_ERR_DLGNRMPROC2));
		}

		return true;
	}
	catch (const basis::message_error& e) {
		TopErrorMessage(nullptr, e.message().data());

		return false;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     プロセスハンドラ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief エディタプロセスを初期化する
	
	CEditWndを作成する。
	
	@author aroka
	@date 2002/01/07

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2004.05.13 Moca CEditWnd::Create()に失敗した場合にfalseを返すように．
	@date 2007.06.26 ryoji グループIDを指定して編集ウィンドウを作成する
	@date 2012.02.25 novice 複数ファイル読み込み
*/
bool CNormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, L"NormalProcess::Init" );

	const auto pszProfileName = CCommandLine::getInstance()->GetProfileName();

	/* プロセス初期化の目印 */
	HANDLE	hMutex = _GetInitializeMutex();	// 2002/2/8 aroka 込み入っていたので分離
	if( NULL == hMutex ){
		return false;
	}

	// コントロールプロセスが起動していなければ起動する
	if (!CControlTray::Find(pszProfileName) && !CNormalProcess::StartControlProcess(pszProfileName)) {
		return false;
	}

	/* 共有メモリを初期化する */
	if ( !CProcess::InitializeProcess() ){
		return false;
	}

	/* 言語を選択する */
	CSelectLang::ChangeLang( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );

	auto fi = CCommandLine::getInstance()->GetEditInfoRef();
	auto gi = CCommandLine::getInstance()->GetGrepInfoRef();

	// 複数ファイルの読み込み
	if (OpenFiles(fi, CCommandLine::getInstance()->GetFiles())) {
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return false;
	}

	/* コマンドラインで受け取ったファイルが開かれている場合は */
	/* その編集ウィンドウをアクティブにする */
	if( fi.m_szPath[0] != L'\0' ){
		//	Oct. 27, 2000 genta
		//	MRUからカーソル位置を復元する操作はCEditDoc::FileLoadで
		//	行われるのでここでは必要なし．

		HWND hwndOwner;
		/* 指定ファイルが開かれているか調べる */
		// 2007.03.13 maru 文字コードが異なるときはワーニングを出すように
		if( GetShareData().ActiveAlreadyOpenedWindow( fi.m_szPath, &hwndOwner, fi.m_nCharCode ) ){
			//	From Here Oct. 19, 2001 genta
			//	カーソル位置が引数に指定されていたら指定位置にジャンプ
			if( fi.m_ptCursor.y >= 0 ){	//	行の指定があるか
				CLogicPoint& pt = GetDllShareData().m_sWorkBuffer.m_LogicPoint;
				if( fi.m_ptCursor.x < 0 ){
					//	桁の指定が無い場合
					::SendMessageAny( hwndOwner, MYWM_GETCARETPOS, 0, 0 );
				}
				else {
					pt.x = fi.m_ptCursor.x;
				}
				pt.y = fi.m_ptCursor.y;
				::SendMessageAny( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
			}
			//	To Here Oct. 19, 2001 genta
			/* アクティブにする */
			ActivateFrameWindow( hwndOwner );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );

			SetSyncEvent();

			return false;
		}
	}

	auto bGrepDlg         = CCommandLine::getInstance()->IsGrepDlg();

	const auto bDebugMode = CCommandLine::getInstance()->IsDebugMode();
	const auto bGrepMode  = CCommandLine::getInstance()->IsGrepMode() || bGrepDlg;

	// プラグイン読み込み
	MY_TRACETIME( cRunningTimer, L"Before Init Jack" );
	/* ジャック初期化 */
	CJackManager::getInstance();
	MY_TRACETIME( cRunningTimer, L"After Init Jack" );

	MY_TRACETIME( cRunningTimer, L"Before Load Plugins" );
	/* プラグイン読み込み */
	CPluginManager::getInstance()->LoadAllPlugin();
	MY_TRACETIME( cRunningTimer, L"After Load Plugins" );

	// エディタアプリケーションを作成。2007.10.23 kobake
	// グループIDを取得
	int nGroupId = CCommandLine::getInstance()->GetGroupId();
	if( GetDllShareData().m_Common.m_sTabBar.m_bNewWindow && nGroupId == -1 ){
		nGroupId = CAppNodeManager::getInstance()->GetFreeGroupId();
	}
	// CEditAppを作成
	m_pcEditApp = CEditApp::getInstance();
	m_pcEditApp->Create(GetProcessInstance(), nGroupId);
	CEditWnd* pEditWnd = m_pcEditApp->GetEditWindow();

	bGrepDlg = ApplyGrepOptions(pEditWnd->m_cDlgGrep);

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	if (gi.bGrepStdout && !bGrepDlg) {
		// Grep実行
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			&pEditWnd->GetActiveView(),
			pEditWnd->m_cDlgGrep
		);
		return true;
	}

	// 編集ウインドウを作成する
	const auto hWndEditor = pEditWnd->Create(nGroupId);
	if (!hWndEditor) {
		return false;	// 2009.06.23 ryoji CEditWnd::Create()失敗のため終了
	}

	SetMainWindow(hWndEditor);

	//	YAZAKI 2002/05/30 IMEウィンドウの位置がおかしいのを修正。
	pEditWnd->GetActiveView().SetIMECompFormPos();

	//WM_SIZEをポスト
	{	// ファイル読み込みしなかった場合にはこの WM_SIZE がアウトライン画面を配置する
		HWND hEditWnd = pEditWnd->GetHwnd();
		if( !::IsIconic( hEditWnd ) ){
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::PostMessageAny( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	//再描画
	::InvalidateRect( pEditWnd->GetHwnd(), NULL, TRUE );

	//プラグイン：EditorStartイベント実行
	CJackManager::getInstance()->InvokePlugins(PP_EDITOR_START, &pEditWnd->GetActiveView());

	if (!bDebugMode || !bGrepMode) {
		// オープン後自動実行マクロを実行する
		pEditWnd->GetDocument()->RunAutoMacro(GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened);

		// 起動時マクロオプション
		if (const auto pszMacro = CCommandLine::getInstance()->GetMacro(); pszMacro && *pszMacro && pEditWnd->GetHwnd()) {
			auto pszMacroType = CCommandLine::getInstance()->GetMacroType();
			if (!pszMacroType || !*pszMacroType || 0 == _wcsicmp(pszMacroType, L"file")) {
				pszMacroType = nullptr;
			}
			pEditWnd->GetActiveView().GetCommander().HandleCommand(F_EXECEXTMACRO, true, LPARAM(pszMacro), LPARAM(pszMacroType), 0, 0);
		}
	}

	if (!bGrepMode) {
		//プラグイン：DocumentOpenイベント実行
		CJackManager::getInstance()->InvokePlugins( PP_DOCUMENT_OPEN, &pEditWnd->GetActiveView() );
	}

	return hWndEditor;
}

/*!
	@brief エディタプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::MainLoop()
{
	if( GetMainWindow() ){
		m_pcEditApp->GetEditWindow()->MessageLoop();	/* メッセージループ */
		return true;
	}
	return false;
}

/*!
	@brief エディタプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	こいつはなにもしない。後始末はdtorで。
*/
void CNormalProcess::OnExitProcess()
{
	/* プラグイン解放 */
	CPluginManager::getInstance()->UnloadAllPlugin();		// Mpve here	2010/7/11 Uchi
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief Mutex(プロセス初期化の目印)を取得する

	多数同時に起動するとウィンドウが表に出てこないことがある。
	
	@date 2002/2/8 aroka InitializeProcessから移動
	@retval Mutex のハンドルを返す
	@retval 失敗した時はリリースしてから NULL を返す
*/
HANDLE CNormalProcess::_GetInitializeMutex() const
{
	MY_RUNNINGTIMER( cRunningTimer, L"NormalProcess::_GetInitializeMutex" );
	HANDLE hMutex;
	const auto pszProfileName = CCommandLine::getInstance()->GetProfileName();
	std::wstring strMutexInitName = GSTR_MUTEX_SAKURA_INIT;
	strMutexInitName += pszProfileName;
	hMutex = ::CreateMutex( NULL, TRUE, strMutexInitName.c_str() );
	if( NULL == hMutex ){
		ErrorBeep();
		TopErrorMessage( NULL, L"CreateMutex()失敗。\n終了します。" );
		return NULL;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		DWORD dwRet = ::WaitForSingleObject( hMutex, 15000 );	// 2002/2/8 aroka 少し長くした
		if( WAIT_TIMEOUT == dwRet ){// 別の誰かが起動中
			TopErrorMessage( NULL, L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。" );
			::CloseHandle( hMutex );
			return NULL;
		}
	}
	return hMutex;
}

/*!
 * @brief 複数ファイル読み込み
 *
 * @retval true 複数ファイルを開いた
 * @retval false ファイルを開かなかった
 * @date 2015.03.14 novice 新規作成
 */
bool CNormalProcess::OpenFiles(EditInfo& fi, const std::vector<std::wstring>& files) const
{
	// 2つ目以降のファイルが指定されていない
	if (files.empty()) {
		return false;
	}

	const auto bViewMode = CCommandLine::getInstance()->IsViewMode();
	if (!CControlTray::OpenNewEditor2(GetProcessInstance(), nullptr, &fi, bViewMode)) {
		return true;	//ファイルを開いた（一部のファイルオープンに失敗した）
	}

	for (const auto&file : files) {
		// ファイル名差し替え
		fi.m_szPath = file.c_str();
		if (!CControlTray::OpenNewEditor2(GetProcessInstance(), nullptr, &fi, bViewMode)) {
			return true;	//ファイルを開いた（一部のファイルオープンに失敗した）
		}
	}

	return true;	//すべてのファイルを開いた
}

/*!
 * @brief Grepオプションをダイアログに適用する
 *
 * @param cDlgGrep Grepダイアログ
 * @return true:  Grepダイアログを表示する必要がある
 * @return false: Grepダイアログを表示する必要はない
 */
bool CNormalProcess::ApplyGrepOptions(CDlgGrep& cDlgGrep) const noexcept {
	auto bGrepDlg         = CCommandLine::getInstance()->IsGrepDlg();

	const auto bGrepMode  = CCommandLine::getInstance()->IsGrepMode() || bGrepDlg;

	if (bGrepMode) {
		const auto& gi = CCommandLine::getInstance()->GetGrepInfoRef();

		auto& sSearch = GetDllShareData().m_Common.m_sSearch;

		sSearch.m_bGrepSubFolder        = gi.bGrepSubFolder;
		sSearch.m_sSearchOption         = gi.sGrepSearchOption;
		sSearch.m_nGrepCharSet          = gi.nGrepCharSet;
		sSearch.m_nGrepOutputLineType   = gi.nGrepOutputLineType;
		sSearch.m_nGrepOutputStyle      = gi.nGrepOutputStyle;
		sSearch.m_bGrepOutputFileOnly   = gi.bGrepOutputFileOnly;
		sSearch.m_bGrepOutputBaseFolder = gi.bGrepOutputBaseFolder;
		sSearch.m_bGrepSeparateFolder   = gi.bGrepSeparateFolder;

		auto& sSearchKeywords = GetDllShareData().m_sSearchKeywords;

		if (const auto pszGrepKey = gi.GetGrepKey(); pszGrepKey && *pszGrepKey && wcslen(pszGrepKey) < _MAX_PATH) {
			cDlgGrep.m_strText = pszGrepKey;
			cDlgGrep.m_bSetText = true;
		}

		if (const auto pszGrepFile = gi.GetGrepFile(); pszGrepFile && *pszGrepFile && wcslen(pszGrepFile) <= MAX_GREP_PATH) {
			cDlgGrep.m_szFile = pszGrepFile;
		}
		else if (!bGrepDlg && !pszGrepFile && sSearchKeywords.m_aGrepFiles.size()) {
			cDlgGrep.m_szFile = sSearchKeywords.m_aGrepFiles[0];
		}

		if (const auto pszGrepFolder = gi.GetGrepFolder(); pszGrepFolder && *pszGrepFolder && wcslen(pszGrepFolder) <= MAX_GREP_PATH) {
			cDlgGrep.m_szFolder = pszGrepFolder;
		}
		else if (!bGrepDlg && !pszGrepFolder && sSearchKeywords.m_aGrepFolders.size()) {
			cDlgGrep.m_szFolder = sSearchKeywords.m_aGrepFolders[0];
		}
		cDlgGrep.m_bGrepStdout				= gi.bGrepStdout;
		cDlgGrep.m_bGrepHeader				= gi.bGrepHeader;
		cDlgGrep.m_bSubFolder				= sSearch.m_bGrepSubFolder;			// Grep: サブフォルダーも検索
		cDlgGrep.m_sSearchOption			= sSearch.m_sSearchOption;				// 検索オプション
		cDlgGrep.m_nGrepCharSet				= sSearch.m_nGrepCharSet;				// 文字コードセット
		cDlgGrep.m_nGrepOutputLineType		= sSearch.m_nGrepOutputLineType;		// 行を出力/該当部分/否マッチ行 を出力
		cDlgGrep.m_nGrepOutputStyle			= sSearch.m_nGrepOutputStyle;			// Grep: 出力形式
		cDlgGrep.m_bGrepOutputFileOnly		= sSearch.m_bGrepOutputFileOnly;
		cDlgGrep.m_bGrepOutputBaseFolder	= sSearch.m_bGrepOutputBaseFolder;
		cDlgGrep.m_bGrepSeparateFolder		= sSearch.m_bGrepSeparateFolder;

		if (!bGrepDlg) {
			bGrepDlg = cDlgGrep.m_strText.empty() || cDlgGrep.m_szFile.empty() || cDlgGrep.m_szFolder.empty();
		}
	}

	return bGrepDlg;
}