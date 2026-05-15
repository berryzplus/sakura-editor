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
	Copyright (C) 2018-2026, Sakura Editor Organization

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

//-------------------------------------------------

/*!
	@brief iniファイルパスを取得する
 */
std::filesystem::path CControlProcess::GetIniFileName() const
{
	if (GetShareDataPtr()->IsPrivateSettings()) {
		return CProcess::GetIniFileName();
	}

	// exe基準のiniファイルパスを得る
	auto iniPath = GetExeFileName().replace_extension(L".ini");

	// マルチユーザー用のiniファイルパス
	//		exeと同じフォルダーに置かれたマルチユーザー構成設定ファイル（sakura.exe.ini）の内容
	//		に従ってマルチユーザー用のiniファイルパスを決める
	auto exeIniPath = GetExeFileName().concat(L".ini");
	if (bool isMultiUserSeggings = ::GetPrivateProfileInt(L"Settings", L"MultiUser", 0, exeIniPath.c_str()); isMultiUserSeggings) {
		return GetPrivateIniFileName(exeIniPath, iniPath.filename());
	}

	const auto filename = iniPath.filename();
	iniPath.remove_filename();

	if (const auto pszProfileName = GetProfileName(); *pszProfileName) {
		iniPath.append(pszProfileName);
	}

	return iniPath.append(filename.c_str());
}

/*!
	@brief マルチユーザー用のiniファイルパスを取得する
 */
std::filesystem::path CControlProcess::GetPrivateIniFileName(const std::wstring& exeIniPath, const std::wstring& filename) const
{
	const auto nFolder = ::GetPrivateProfileInt(L"Settings", L"UserRootFolder", 0, exeIniPath.c_str());
	KNOWNFOLDERID refFolderId;
	switch (nFolder) {
	case 1:
	case 3:
		refFolderId = FOLDERID_Profile;			// ユーザーのルートフォルダー
		break;
	case 2:
		refFolderId = FOLDERID_Documents;		// ユーザーのドキュメントフォルダー
		break;

	default:
		refFolderId = FOLDERID_RoamingAppData;	// ユーザーのアプリケーションデータフォルダー
		break;
	}

	PWSTR pFolderPath = nullptr;
	::SHGetKnownFolderPath(refFolderId, KF_FLAG_DEFAULT_PATH, nullptr, &pFolderPath);
	std::filesystem::path privateIniPath(pFolderPath);
	::CoTaskMemFree(pFolderPath);

	std::wstring subFolder(_MAX_DIR, L'\0');
	::GetPrivateProfileString(L"Settings", L"UserSubFolder", L"sakura", subFolder.data(), (DWORD)subFolder.capacity(), exeIniPath.c_str());
	subFolder.assign(subFolder.data());
	if (subFolder.empty())
	{
		subFolder = L"sakura";
	}
	if (nFolder == 3) {
		privateIniPath.append("Desktop");
	}
	privateIniPath.append(subFolder);

	if (const auto pszProfileName = GetProfileName(); *pszProfileName) {
		privateIniPath.append(pszProfileName);
	}

	return privateIniPath.append(filename.c_str());
}

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
bool CControlProcess::InitializeProcess(int nCmdShow)
{
	MY_RUNNINGTIMER( cRunningTimer, L"CControlProcess::InitializeProcess" );

	if (!*m_ComInit) return false;

	// アプリケーション実行検出用(インストーラで使用)
	m_hMutex = ::CreateMutex( nullptr, FALSE, GSTR_MUTEX_SAKURA );
	if( nullptr == m_hMutex ){
		ErrorBeep();
		TopErrorMessage( nullptr, L"CreateMutex()失敗。\n終了します。" );
		return false;
	}

	const auto pszProfileName = GetProfileName();

	// 初期化完了イベントを作成する
	std::wstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += pszProfileName;
	m_hEventCPInitialized = ::CreateEvent( nullptr, TRUE, FALSE, strInitEvent.c_str() );
	if( nullptr == m_hEventCPInitialized )
	{
		ErrorBeep();
		TopErrorMessage( nullptr, L"CreateEvent()失敗。\n終了します。" );
		return false;
	}

	/* コントロールプロセスの目印 */
	std::wstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += pszProfileName;
	m_hMutexCP = ::CreateMutex( nullptr, TRUE, strCtrlProcEvent.c_str() );
	if( nullptr == m_hMutexCP ){
		ErrorBeep();
		TopErrorMessage( nullptr, L"CreateMutex()失敗。\n終了します。" );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		return false;
	}
	
	/* 共有メモリを初期化 */
	if (!GetShareData().InitShareData()) {
		return false;
	}

	// コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
	WCHAR szDir[_MAX_PATH];
	::GetSystemDirectory( szDir, int(std::size(szDir)) );
	::SetCurrentDirectory( szDir );

	/* 共有データのロード */
	if( !CShareData_IO::LoadShareData() ){
		/* レジストリ項目 作成 */
		CShareData_IO::SaveShareData();
	}

	auto pClassFactory = cxx::CreateControlClassFactory();

	cxx::com_pointer<ITrayWnd> pTrayWnd;
	if (FAILED(pClassFactory->CreateInstance(nullptr, IID_PPV_ARGS(&pTrayWnd)))) return false;

	m_pcTray = std::make_unique<CControlTray>(*pTrayWnd);

	/* 言語を選択する */
	CSelectLang::ChangeLang(GetDllShareData().m_Common.m_sWindow.m_szLanguageDll);

	RefreshString();

	/* タスクトレイにアイコン作成 */
	const auto hwnd = m_pcTray->CreateMainWnd(GetProcessInstance(), nCmdShow);
	if( !hwnd ){
		ErrorBeep();
		TopErrorMessage( nullptr, LS(STR_ERR_CTRLMTX3) );
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_sHandles.m_hwndTray = hwnd;

	// 初期化完了イベントをシグナル状態にする
	if( !::SetEvent( m_hEventCPInitialized ) ){
		ErrorBeep();
		TopErrorMessage( nullptr, LS(STR_ERR_CTRLMTX4) );
		return false;
	}

	return true;
}

/*!
	@brief コントロールプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
int CControlProcess::MainLoop() const
{
	return m_pcTray->MessageLoop();
}
