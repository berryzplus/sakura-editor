/*!	@file
	@brief 常駐部
	
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, Stonee, jepro, genta, aroka, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, YAZAKI, towest
	Copyright (C) 2003, MIK, Moca, KEITA, genta, aroka
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CControlTray.h"

#include "typeprop/CDlgTypeList.h"
#include "debug/CRunningTimer.h"
#include "dlg/CDlgOpenFile.h"
#include "dlg/CDlgAbout.h"		//Nov. 21, 2000 JEPROtest
#include "dlg/CDlgFavorite.h"
#include "dlg/CDlgWindowList.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "env/CShareData.h"
#include "env/CShareData_IO.h"
#include "env/CSakuraEnvironment.h"
#include "env/CHelpManager.h"
#include "doc/CDocListener.h" // SLoadInfo,EditInfo
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "_main/CCommandLine.h"
#include "grep/CGrepEnumKeys.h"
#include "apiwrap/StdApi.h"
#include "sakura_rc.h"
#include "config/app_constants.h"
#include "apiwrap/DarkMode.h"

#define ID_HOTKEY_TRAYMENU	0x1234

#define IDT_EDITCHECK 2
// 3秒
#define IDT_EDITCHECK_INTERVAL 3000

WORD convertHotKeyMods(WORD wHotKeyMods) noexcept
{
	WORD wMods = 0;
	if (HOTKEYF_SHIFT & wHotKeyMods) {
		wMods |= MOD_SHIFT;
	}
	if (HOTKEYF_CONTROL & wHotKeyMods) {
		wMods |= MOD_CONTROL;
	}
	if (HOTKEYF_ALT & wHotKeyMods) {
		wMods |= MOD_ALT;
	}
	return wMods;
}

namespace window {

//先行宣言
bool SendTrayMessage(
	_In_ HWND hWndTray,
	_In_ UINT uID,
	_In_ DWORD dwMessage,
	_In_opt_ HICON hIcon = nullptr,
	const std::optional<std::wstring>& optTipText = std::nullopt
);

bool ActivateOpenedEditor(int openEditorIndex)
{
	// 開いているエディターをアクティブにする
	const auto& pEditArr = GetDllShareData().m_sNodes.m_pEditArr;

	if (openEditorIndex < 0 || std::ssize(pEditArr) <= openEditorIndex) return false;

	const auto safeSize = std::max(0, GetDllShareData().m_sNodes.m_nEditArrNum);
	const auto actualSize = std::min<size_t>(safeSize, std::size(pEditArr));

	if (actualSize <= openEditorIndex) return false;

	const auto nodes = std::span{ pEditArr, actualSize };

	const auto hEditWnd = nodes[openEditorIndex].GetHwnd();

	ActivateFrameWindow(hEditWnd);

	return true;
}

/*!
 * @brief タスクトレイにアイコンを作成する
 *
 * @param hInstance [in] インスタンスハンドル
 * @param hWndTray [in] タスクトレイのウィンドウハンドル
 * @retval true 作成に成功
 * @retval false 作成に失敗
 *
 * @date 2001/01/12 JEPRO トレイアイコンにポイントするとバージョンno.が表示されるように修正
 * @date 2001/04/12 aroka
 */
bool CreateTrayIcon(
	_In_ HINSTANCE hInstance,
	_In_ HWND hWndTray
)
{
	// タスクトレイのアイコンを使わない場合、直ちに抜ける
	if (!GetDllShareData().m_Common.m_sGeneral.m_bUseTaskTray) return false;

	//アイコンを読み込む
	const auto hIcon = GetAppIcon(hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true);

	//バージョン情報を取得
	DWORD dwVersionMS;
	DWORD dwVersionLS;
	GetAppVersionInfo(hInstance, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS);

	//プロファイル名を取得
	std::wstring profileName{ GetProfileName() };

	//ツールチップテキストを組み立てる
	const auto tipText = std::format(L"{:s} {:d}.{:d}.{:d}.{:d}{:s}",		//Jul. 06, 2001 jepro UR はもう付けなくなったのを忘れていた
		GSTR_APPNAME,
		HIWORD(dwVersionMS),
		LOWORD(dwVersionMS),
		HIWORD(dwVersionLS),
		LOWORD(dwVersionLS),
		profileName.empty() ? L"" : std::format(L" {:s}", profileName)
	);

	//タスクトレイにメッセージを送信する
	return window::SendTrayMessage(hWndTray, 0, NIM_ADD, hIcon, tipText);
}

bool OpenSelectedMruFile(int mruIndex)
{
	if (mruIndex < 0 || 999 <= mruIndex) return false;

	EditInfo openEditInfo;
	if (!CMRUFile().GetEditInfo(mruIndex, &openEditInfo)) return false;

	HINSTANCE unusedArg1 = nullptr;
	HWND unusedArg2 = nullptr;

	if (GetDllShareData().m_Common.m_sFile.GetRestoreCurPosition()) {
		CControlTray::OpenNewEditor2(unusedArg1, unusedArg2, &openEditInfo, false);
	}
	else {
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = openEditInfo.m_szPath;
		sLoadInfo.eCharCode = openEditInfo.m_nCharCode;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor(unusedArg1, unusedArg2, sLoadInfo, L"", true, nullptr, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow);
	}

	return true;
}

void SelectAndOpenFiles(
	const std::filesystem::path& defaultDir,
	const std::vector<LPCWSTR>& vOPENFOLDER
)
{
	/* ファイルオープンダイアログの初期化 */
	CDlgOpenFile cDlgOpenFile;
	cDlgOpenFile.Create(
		HINSTANCE(nullptr),
		HWND(nullptr),
		L"*.*",
		defaultDir.c_str(),
		CMRUFile().GetPathList(),
		vOPENFOLDER
	);
	SLoadInfo sLoadInfo( L"", CODE_AUTODETECT, false);
	std::vector<std::wstring> files;
	if (!cDlgOpenFile.DoModalOpenDlg(&sLoadInfo, &files)) {
		return;
	}

	// 新たな編集ウィンドウを起動
	size_t nSize = files.size();
	for( size_t f = 0; f < nSize; f++ ){
		sLoadInfo.cFilePath = files[f].c_str();
		CControlTray::OpenNewEditor(nullptr, nullptr, sLoadInfo, nullptr, true, nullptr, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow);
	}
}

bool SelectAndOpenFilesFromMruFolder(int mruFolderIndex)
{
	if (mruFolderIndex < 0 || 999 <= mruFolderIndex) return false;

	/* OPENFOLDERリストのファイルのリスト */
	auto vOPENFOLDER = CMRUFolder().GetPathList();
	if (std::ssize(vOPENFOLDER) <= mruFolderIndex) return false;
	const auto selectedMruFolder = vOPENFOLDER[mruFolderIndex];

	//Stonee, 2001/12/21 UNCであれば接続を試みる
	NetConnect(selectedMruFolder);

	/* ファイルオープンダイアログの初期化 */
	window::SelectAndOpenFiles(
		selectedMruFolder,
		vOPENFOLDER
	);

	return true;
}

/*!
 * @brief タスクトレイにメッセージを送信する
 *
 * @param hWndTray [in] タスクトレイのウィンドウハンドル
 * @param uID [in] トレイアイコンの識別子(ウィンドウハンドルとセット)
 * @param dwMessage [in] NIM_ADD, NIM_MODIFY, NIM_DELETE のいずれか
 * @param hIcon [in, opt] トレイアイコンのハンドル
 * @param optTipText [in, opt] トレイアイコンのツールチップテキスト
 * @return Shell_NotifyIconWの戻り値
 */
bool SendTrayMessage(
	_In_ HWND hWndTray,
	_In_ UINT uID,
	_In_ DWORD dwMessage,
	_In_opt_ HICON hIcon,
	const std::optional<std::wstring>& optTipText
)
{
	NOTIFYICONDATA tnd{ sizeof(NOTIFYICONDATA) };
	tnd.hWnd				= hWndTray;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;

	if (hIcon) {
		tnd.uFlags |= NIF_ICON;
		tnd.hIcon	= hIcon;
	}

	if (optTipText.has_value()) {
		tnd.uFlags |= NIF_TIP;
		const auto& tipText = *optTipText;
		if (std::size(tnd.szTip) <= tipText.length() + 1) throw std::length_error("Tooltip text is too long");
		std::ranges::copy(tipText, tnd.szTip);
	}

	return ::Shell_NotifyIconW(dwMessage, &tnd);
}

EFunctionCode TrackPopupMenu(
	HMENU hMenu,
	UINT uFlags,
	HWND hWnd
)
{
	POINT pt{};
	::GetCursorPos(&pt);
	pt.y -= DpiScaleY(4);

	::SetActiveWindow(hWnd);
	::SetForegroundWindow(hWnd);

	const auto id = ::TrackPopupMenu(
		hMenu,
		uFlags
		| TPM_RETURNCMD
		,
		pt.x,
		pt.y,
		0,
		hWnd,
		nullptr
	);

	::PostMessageW(hWnd, WM_USER + 1, 0L, 0L);

	return static_cast<EFunctionCode>(id);
}

} // namespace window

struct QueuedCommand
{
	EFunctionCode funcCode = F_0;
	bool bStopRequested = true;

	QueuedCommand() = default;

	explicit QueuedCommand(EFunctionCode funcCode_)
		: funcCode(funcCode_)
		, bStopRequested(false)
	{
	}

	explicit operator bool() const noexcept
	{
		return !bStopRequested;
	}
};

//Stonee, 2001/03/21
//Stonee, 2001/07/01  多重起動された場合は前回のダイアログを前面に出すようにした。
void CControlTray::DoGrep()
{
	m_cDlgGrep.m_bEnableThisText = false;

	//Stonee, 2001/06/30
	//前回のダイアログがあれば前面に (suggested by genta)
	if ( ::IsWindow(m_cDlgGrep.GetHwnd()) ){
		::OpenIcon(m_cDlgGrep.GetHwnd());
		::BringWindowToTop(m_cDlgGrep.GetHwnd());
		return;
	}

	if( 0 < m_pShareData->m_sSearchKeywords.m_aSearchKeys.size()
		&& m_nCurSearchKeySequence < GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence ){
		m_cDlgGrep.m_strText = m_pShareData->m_sSearchKeywords.m_aSearchKeys[0];
	}
	if( 0 < m_pShareData->m_sSearchKeywords.m_aGrepFiles.size() ){
		wcscpy( m_cDlgGrep.m_szFile, m_pShareData->m_sSearchKeywords.m_aGrepFiles[0] );		/* 検索ファイル */
	}
	if( 0 < m_pShareData->m_sSearchKeywords.m_aGrepFolders.size() ){
		wcscpy( m_cDlgGrep.m_szFolder, m_pShareData->m_sSearchKeywords.m_aGrepFolders[0] );	/* 検索フォルダー */
	}
	if (0 < m_pShareData->m_sSearchKeywords.m_aExcludeFiles.size()) {
		wcscpy(m_cDlgGrep.m_szExcludeFile, m_pShareData->m_sSearchKeywords.m_aExcludeFiles[0]);	/* 除外ファイル */
	}
	if (0 < m_pShareData->m_sSearchKeywords.m_aExcludeFolders.size()) {
		wcscpy(m_cDlgGrep.m_szExcludeFolder, m_pShareData->m_sSearchKeywords.m_aExcludeFolders[0]);	/* 除外フォルダー */
	}

	/* Grepダイアログの表示 */
	int nRet = m_cDlgGrep.DoModal( m_hInstance, nullptr, L"" );
	if( !nRet || GetTrayHwnd() == nullptr ){
		return;
	}
	m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;
	DoGrepCreateWindow(m_hInstance, GetDllShareData().m_sHandles.m_hwndTray, m_cDlgGrep);
}

void CControlTray::DoGrepCreateWindow(HINSTANCE hinst, HWND msgParent, CDlgGrep& cDlgGrep)
{
	/*======= Grepの実行 =============*/
	/* Grep結果ウィンドウの表示 */

	CNativeW		cmWork1;
	CNativeW		cmWork2;
	CNativeW		cmWork3;

	cmWork1.SetString( cDlgGrep.m_strText.c_str() );
	cmWork2 = cDlgGrep.GetPackedGFileString();
	cmWork3.SetString( cDlgGrep.m_szFolder );

	cmWork1.Replace( L"\"", L"\"\"" );
	cmWork2.Replace( L"\"", L"\"\"" );
	cmWork3.Replace( L"\"", L"\"\"" );

	// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	CNativeW cCmdLine;
	WCHAR szTemp[20];

	cCmdLine.AppendString(L"-GREPMODE -GKEY=\"");
	cCmdLine.AppendString(cmWork1.GetStringPtr());
	cCmdLine.AppendString(L"\" -GFILE=\"");
	cCmdLine.AppendString(cmWork2.GetStringPtr());
	cCmdLine.AppendString(L"\" -GFOLDER=\"");
	cCmdLine.AppendString(cmWork3.GetStringPtr());
	cCmdLine.AppendString(L"\" -GCODE=");
	auto_sprintf( szTemp, L"%d", cDlgGrep.m_nGrepCharSet );
	cCmdLine.AppendString(szTemp);

	//GOPTオプション
	WCHAR pOpt[64] = L"";
	if( cDlgGrep.m_bSubFolder					)wcscat( pOpt, L"S" );	// サブフォルダーからも検索する
	if( cDlgGrep.m_sSearchOption.bLoHiCase		)wcscat( pOpt, L"L" );	// 英大文字と英小文字を区別する
	if( cDlgGrep.m_sSearchOption.bRegularExp	)wcscat( pOpt, L"R" );	// 正規表現
	if( cDlgGrep.m_nGrepOutputLineType == 1     )wcscat( pOpt, L"P" );	// 行を出力する
	if( cDlgGrep.m_nGrepOutputLineType == 2     )wcscat( pOpt, L"N" );	// 否ヒット行を出力する 2014.09.23
	if( cDlgGrep.m_sSearchOption.bWordOnly		)wcscat( pOpt, L"W" );	// 単語単位で探す
	if( 1 == cDlgGrep.m_nGrepOutputStyle		)wcscat( pOpt, L"1" );	// Grep: 出力形式
	if( 2 == cDlgGrep.m_nGrepOutputStyle		)wcscat( pOpt, L"2" );	// Grep: 出力形式
	if( 3 == cDlgGrep.m_nGrepOutputStyle		)wcscat( pOpt, L"3" );
	if( cDlgGrep.m_bGrepOutputFileOnly		)wcscat( pOpt, L"F" );
	if( cDlgGrep.m_bGrepOutputBaseFolder		)wcscat( pOpt, L"B" );
	if( cDlgGrep.m_bGrepSeparateFolder		)wcscat( pOpt, L"D" );
	if( pOpt[0] != L'\0' ){
		cCmdLine.AppendString( L" -GOPT=" );
		cCmdLine.AppendString( pOpt );
	}

	/* 新規編集ウィンドウの追加 ver 0 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = L"";
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	OpenNewEditor( hinst, msgParent, sLoadInfo, cCmdLine.GetStringPtr(),
		false, nullptr, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow? true : false );
}

/////////////////////////////////////////////////////////////////////////////
// CControlTray
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CControlTray::CControlTray()
	: CAppMainWnd(std::format(L"{:s}{:s}", GSTR_CEDITAPP, GetProfileName()))
{
	// 操作キューを作成する
	SFilePath queueName{ std::format(GSTR_SAKURA_CP_QUEUE, GetProfileName()) };
	m_hQueue = ::CreateSemaphoreW(nullptr, 0, 1, queueName);

	// ワーカーを起動する
	m_Worker = std::jthread([this](std::stop_token st) {
		AsyncCommandProc(st);
	});
}

CControlTray::~CControlTray()
{
	// ワーカーがまだ動いているなら、終了指示を出す
	if (m_Worker.joinable()) {
		m_Worker.request_stop();
		m_QueueCv.notify_one();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CControlTray メンバ関数

void CControlTray::AsyncCommandProc(std::stop_token st)
{
	const auto peek_queue = [this, st]()
	{
		std::unique_lock lock{ m_QueueMutex };

		m_QueueCv.wait( lock, [this, st] {
			return !m_Queue.empty() || st.stop_requested();
		} );

		if (st.stop_requested() || m_Queue.empty()) return QueuedCommand{};

		const auto value = m_Queue.front();

		m_Queue.pop();
		m_QueueCv.notify_one();

		return QueuedCommand{ value };
	};

	while (!st.stop_requested()) {

		// リクエストキューから次の値を取得する
		const auto next = peek_queue();

		if (!next || st.stop_requested()) {
			break;
		}

		// 操作キューのロック取得を試行
		if (const auto dwRet = ::WaitForSingleObject(m_hQueue, 5000); WAIT_OBJECT_0 == dwRet) {
			// コマンドを実行する
			ExecCommand(next.funcCode);

			// 操作キューを解放
			::ReleaseSemaphore(m_hQueue, 1, nullptr);
		}
	}
}

/* 作成 */
HWND CControlTray::CreateMainWnd(
	HINSTANCE hInstance [[maybe_unused]],
	int nCmdShow [[maybe_unused]]
)
{
	MY_RUNNINGTIMER( cRunningTimer, L"CControlTray::Create" );

	//同名同クラスのウィンドウが既に存在していたら、失敗
	if (cxx::FindWindowW(m_ClassName, m_ClassName)) {
		return nullptr;
	}

	//ウィンドウクラス登録
	const auto atom = Base::RegisterClassW(HBRUSH(COLOR_WINDOW + 1), ::LoadCursorW(nullptr, IDC_ARROW), CS_DBLCLKS, 0, ::LoadIconW(nullptr, IDI_APPLICATION));

	if (!atom) {
			ErrorMessage( nullptr, LS(STR_TRAY_CREATE) );
	}

	CMyRect rc;
	rc.SetXYWH(
		CW_USEDEFAULT,	// horizontal position of window
		SW_HIDE,		// vertical position of window
		100,			// window width
		100				// window height
	);

	// ウィンドウ作成 (WM_CREATEで、GetHwnd() に HWND が格納される)
	return Base::CreateWnd(atom, WS_OVERLAPPEDWINDOW, HWND(nullptr), 0, rc, m_ClassName);
}

/*!
 * @brief コマンドを実行する
 */
void CControlTray::ExecCommand(EFunctionCode id)
{
	const auto hWnd = GetHwnd();

	switch (id) {
	case F_FILENEW:	/* 新規作成 */
		/* 新規編集ウィンドウの追加 */
		OnNewEditor(false);	//既存タブグループがあればまとめる
		break;

	case F_FILEOPEN:	/* 開く */
		window::SelectAndOpenFiles(
			CSakuraEnvironment::GetDlgInitialDir(true),
			CMRUFolder().GetPathList()
		);
		break;

	case F_GREP_DIALOG:
		/* Grep */
		DoGrep();  //Stonee, 2001/03/21  Grepを別関数に
		break;

	case F_FAVORITE:
		{
			CDlgFavorite cDlgFavorite;
			cDlgFavorite.DoModal(m_hInstance, hWnd, (LPARAM)nullptr);
		}
		break;

	case F_FILESAVEALL:	// Jan. 24, 2005 genta 全て上書き保存
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			WM_COMMAND,
			MAKELONG( F_FILESAVE_QUIET, 0 ),
			(LPARAM)0,
			nullptr
		);
		break;

	case F_HELP_CONTENTS:
		/* ヘルプ目次 */
		ShowWinHelpContents( GetTrayHwnd() );	//	目次を表示する
		break;

	case F_HELP_SEARCH:
		/* ヘルプキーワード検索 */
		MyWinHelp( GetTrayHwnd(), HELP_KEY, (ULONG_PTR)L"" );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		break;

	case F_EXTHELP1:
		/* 外部ヘルプ１ */
		do{
			if( CHelpManager().ExtWinHelpIsSet() ) {	//	共通設定のみ確認
				break;
			}
			else{
				ErrorBeep();
			}
		}while(IDYES == ::MYMESSAGEBOX(
				nullptr, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
				GSTR_APPNAME,
				LS(STR_TRAY_EXTHELP1))
		);/*do-while*/

		break;

	case F_EXTHTMLHELP:
		/* 外部HTMLヘルプ */
		{
//					CEditView::Command_EXTHTMLHELP();
		}
		break;

	case F_TYPE_LIST:	// タイプ別設定一覧
		{
			CDlgTypeList			cDlgTypeList;
			CDlgTypeList::SResult	sResult;
			sResult.cDocumentType = CTypeConfig(0);
			sResult.bTempChange = false;
			if (cDlgTypeList.DoModal(G_AppInstance(), GetTrayHwnd(), &sResult)) {
				// タイプ別設定
				CPluginManager::getInstance()->LoadAllPlugin();
				m_pcPropertyManager->OpenPropertySheetTypes(nullptr, -1, sResult.cDocumentType);
				CPluginManager::getInstance()->UnloadAllPlugin();
			}
		}
		break;

	case F_OPTION:	// 共通設定
		CPluginManager::getInstance()->LoadAllPlugin();
		// アイコンの登録
		GetIcons().ResetExtend();
		for (const auto plug : CJackManager::getInstance()->GetPlugs(PP_COMMAND)) {
			const auto iBitmap = plug->m_sIcon.empty()
				? CMenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1
				: GetIcons().Add(plug->m_cPlugin.GetFilePath( plug->m_sIcon ).c_str());

			m_cMenuDrawer.AddToolButton( iBitmap, plug->GetFunctionCode() );
		}
		// 共通設定プロフパティシートの表示は、たまにクラッシュして戻らない。
		m_pcPropertyManager->OpenPropertySheet( nullptr, -1, true );
		CPluginManager::getInstance()->UnloadAllPlugin();
		break;

	case F_ABOUT:
		/* バージョン情報 */
		{
			CDlgAbout cDlgAbout;
			cDlgAbout.DoModal(m_hInstance, hWnd);
		}
		break;

	case F_EXITALLEDITORS:
		/* 編集の全終了 */
		CloseAllEditor(TRUE, hWnd, TRUE, 0);
		break;

	case F_EXITALL:
		/* サクラエディタの全終了 */
		TerminateApplication(hWnd);
		break;

	default:
		window::ActivateOpenedEditor(static_cast<int>(id) - IDM_SELWINDOW) ||
			window::OpenSelectedMruFile(static_cast<int>(id) - IDM_SELMRU) ||
			window::SelectAndOpenFilesFromMruFolder(static_cast<int>(id) - IDM_SELOPENFOLDER);
		break;
	}
}

/*!
 * @brief メッセージループ
 *
 * @return PostQuitMessage()で指定された終了コード
 */
int CControlTray::MessageLoop() const
{
	MSG	msg{};

	while (::GetMessageW(&msg, nullptr, 0, 0)) {
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}

	return static_cast<int>(msg.wParam);
}

void CControlTray::PushCommand(EFunctionCode funcCode)
{
	std::unique_lock lock{ m_QueueMutex };

	m_Queue.push( funcCode );

	m_QueueCv.notify_one();
}

//! ホットキーを登録する
void CControlTray::RegisterHotKey(HWND hWnd) noexcept
{
	wHotKeyMods = convertHotKeyMods(m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyMods);
	wHotKeyCode = m_pShareData->m_Common.m_sGeneral.m_wTrayMenuHotKeyCode;

	if (wHotKeyCode) {
		// タスクトレイ左クリックメニューへのショートカットキー登録
		::RegisterHotKey(
			hWnd,
			ID_HOTKEY_TRAYMENU,
			wHotKeyMods,
			wHotKeyCode
		);
	}
}

/*!
 * @brief トレイウインドウのメッセージ配送
 *
 * @param hWnd [in] 宛先ウインドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 *
 * @date 2001/12/26 YAZAKI MRUリストは、CMRUに依頼する
 */
LRESULT CControlTray::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	const auto hWnd = hwnd;

	int			nRowNum;
	EditNode*	pEditNodeArr;

	switch (uMsg) {
// clang-format off
	HANDLE_MSG(hWnd, WM_CREATE,							OnCreate);
	HANDLE_MSG(hWnd, WM_DESTROY,						OnDestroy);
	HANDLE_MSG(hWnd, WM_CLOSE,							OnClose);
	HANDLE_MSG(hWnd, WM_TIMER,							OnTimer);
	HANDLE_MSG(hWnd, WM_HOTKEY,							OnHotKey);
// clang-format on

	case WM_QUERYENDSESSION:
		return OnQueryEndSession(hWnd, UINT(lParam));

	case WM_ENDSESSION:
		OnEndSession(hWnd, wParam, UINT(lParam));
		return 0;	//	もうこのプロセスに制御が戻ることはない

	case WM_HELP:
		OnHelp(hWnd, LPHELPINFO(lParam));
		return TRUE;

	case WM_COMMAND:
		return 0L;	//何もしない

	case WM_MENUCHAR:
		/* メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
		return m_cMenuDrawer.OnMenuChar( hwnd, uMsg, wParam, lParam );

	default:
		break;
	}

	switch (uMsg) {
	case MYWM_UIPI_CHECK:
		/* エディタ－トレイ間でのUI特権分離の確認メッセージ */	// 2007.06.07 ryoji
		::SendMessageW(HWND(lParam), MYWM_UIPI_CHECK, 0L, LPARAM(hWnd));	// 返事を返す
		return lParam;

	case MYWM_HTMLHELP:
		{
			auto &sWorkBuffer = m_pShareData->m_sWorkBuffer;
			WCHAR* pWork = sWorkBuffer.GetWorkBuffer<WCHAR>();

			// pszHelpFile取得
			const WCHAR* pszHelpFile = pWork;
			const size_t cchHelpFile = wcsnlen( pWork, sWorkBuffer.GetWorkBufferCount<WCHAR>() );

			// pszKeywords取得
			const WCHAR* pszKeywords = &pWork[cchHelpFile + 1];

			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				nullptr,
				pszHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD_PTR)0,
				true
			);

			HH_AKLINK	link;
			link.cbStruct		= sizeof_raw(link);
			link.fReserved		= FALSE;
			link.pszKeywords	= pszKeywords;
			link.pszUrl			= nullptr;
			link.pszMsgText		= nullptr;
			link.pszMsgTitle	= nullptr;
			link.pszWindow		= nullptr;
			link.fIndexOnFail	= TRUE;

			//	Jul. 6, 2001 genta HtmlHelpの呼び出し方法変更
			hwndHtmlHelp = OpenHtmlHelp(
				nullptr,
				pszHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link,
				false
			);
		}
		return (LRESULT)hwndHtmlHelp;

	/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
	case MYWM_DELETE_ME:
		// タスクトレイのアイコンを常駐しない、または、トレイにアイコンを作っていない
		if( !(m_pShareData->m_Common.m_sGeneral.m_bStayTaskTray && m_pShareData->m_Common.m_sGeneral.m_bUseTaskTray) || !m_bCreatedTrayIcon ){
			// 現在開いている編集窓のリスト
			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if( 0 < nRowNum ){
				delete [] pEditNodeArr;
			}
			// 編集ウィンドウの数が0になったら終了
			if( 0 == nRowNum ){
				::SendMessage( hwnd, WM_CLOSE, 0, 0 );
			}
		}
		return 0;

	case MYWM_DLGWINLIST:
		{
			static CDlgWindowList dlg;
			if (dlg.GetHwnd() == nullptr) {
				dlg.DoModal(m_hInstance, hwnd, 0);
			}else{
				::SetForegroundWindow(dlg.GetHwnd());
				::BringWindowToTop(dlg.GetHwnd());
			}
		}
		return 0;

	case MYWM_CHANGESETTING:
		switch( (e_PM_CHANGESETTING_SELECT)lParam ){
		case PM_CHANGESETTING_ALL:
			/* ダークモード設定を反映する（変更時のみ適用） */
			if( (GetDllShareData().m_Common.m_sWindow.m_bDarkMode != FALSE) != IsDarkModeActive() ){
				ApplyDarkModeSetting(GetDllShareData().m_Common.m_sWindow.m_bDarkMode);
			}
			{
				bool bChangeLang = wcscmp( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll, m_szLanguageDll ) != 0;
				wcscpy( m_szLanguageDll, GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );
				std::vector<std::wstring> values;
				if( bChangeLang ){
					CShareData::getInstance()->ConvertLangValues(values, true);
				}
				/* 言語を選択する */
				CSelectLang::ChangeLang( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );
				if( bChangeLang ){
					CShareData::getInstance()->ConvertLangValues(values, false);
				}
			}

			::UnregisterHotKey( GetTrayHwnd(), ID_HOTKEY_TRAYMENU );

			// タスクトレイ左クリックメニューへのショートカットキー登録
			RegisterHotKey(hWnd);

			break;
		default:
			break;
		}
		return 0L;

	case MYWM_NOTIFYICON:
//		MYTRACE( L"MYWM_NOTIFYICON\n" );
		switch (lParam){
//キーワード：トレイ右クリックメニュー設定
//	From Here Oct. 12, 2000 JEPRO 左右とも同一処理になっていたのを別々に処理するように変更
		case WM_RBUTTONUP:	// Dec. 24, 2002 towest UPに変更
			/* ポップアップメニュー(トレイ右ボタン) */
			if (const auto eFuncCode = TrackPopupMenu_R(hWnd); F_0 != eFuncCode) {
				PushCommand(eFuncCode);
			}
			// 操作キューを解放
			::ReleaseSemaphore(m_hQueue, 1, nullptr);
			return 0L;
//	To Here Oct. 12, 2000

		case WM_LBUTTONDOWN:
			//	Mar. 29, 2003 genta 念のためフラグクリア
			bLDClick = false;
			return 0L;

		case WM_LBUTTONUP:	// Dec. 24, 2002 towest UPに変更
//			MYTRACE( L"WM_LBUTTONDOWN\n" );
			/* 03/02/20 左ダブルクリック後はメニューを表示しない ai Start */
			if( bLDClick ){
				bLDClick = false;
				return 0L;
			}
			/* 03/02/20 ai End */
			/* ポップアップメニュー(トレイ左ボタン) */
			if (const auto eFuncCode = TrackPopupMenu_L(hWnd); F_0 != eFuncCode) {
				PushCommand(eFuncCode);
			}
			// 操作キューを解放
			::ReleaseSemaphore(m_hQueue, 1, nullptr);
			return 0L;

		case WM_LBUTTONDBLCLK:
			bLDClick = true;		/* 03/02/20 ai */
			/* 新規編集ウィンドウの追加 */
			OnNewEditor( m_pShareData->m_Common.m_sTabBar.m_bNewWindow != FALSE );
			// Apr. 1, 2003 genta この後で表示されたメニューは閉じる
			::PostMessageAny( GetTrayHwnd(), WM_CANCELMODE, 0, 0 );
			return 0L;

		case WM_RBUTTONDBLCLK:
			return 0L;

		default:
			break;
		}
		break;

	case MYWM_ALLOWACTIVATE:
		::AllowSetForegroundWindow(DWORD(wParam));
		return 0L;

	case MYWM_SET_TYPESETTING:
		return OnSetTypeSetting(wParam);

	case MYWM_GET_TYPESETTING:
		return OnGetTypeSetting(wParam);

	case MYWM_ADD_TYPESETTING:
		return OnAddTypeSetting(wParam);

	case MYWM_DEL_TYPESETTING:
		return OnDelTypeSetting(wParam);

	default:
		// タスクバーが再作成されたときは、トレイアイコンを再登録する
		if (gm_uMsgTaskbarCreated == uMsg) {
			m_bCreatedTrayIcon = window::CreateTrayIcon(m_hInstance, hWnd);
			break;	//あとはデフォルトに任せる
		}
		break;	/* default */
	}

	//あとはデフォルトに任せる
	return Base::DispatchEvent(hWnd, uMsg, wParam, lParam);
}

/*!
 * WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 * メッセージの戻り値はウインドウの作成を続行するかどうかの判断に使われます。
 *
 * @retval true  ウィンドウの作成を続行する
 * @retval false ウィンドウの作成を中止する
 */
bool CControlTray::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	if (!Base::OnCreate(hWnd, lpCreateStruct)) {
		return false;
	}

	m_pShareData->m_sHandles.m_hwndTray = hWnd;

	m_szLanguageDll = m_pShareData->m_Common.m_sWindow.m_szLanguageDll;

	// タスクトレイアイコン作成
	m_bCreatedTrayIcon = window::CreateTrayIcon(m_hInstance, hWnd);

	// タスクトレイ左クリックメニューへのショートカットキー登録
	RegisterHotKey(hWnd);

	// 最後の方でシャットダウンするアプリケーションにする
	::SetProcessShutdownParameters(0x180, 0);

	// 操作キューを利用可能にする
	::ReleaseSemaphore(m_hQueue, 1, nullptr);

	// 最前面にする（トレイからのポップアップウィンドウが最前面になるように）
	::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// 編集ウィンドウの終了チェックを開始する
	::SetTimer(hWnd, IDT_EDITCHECK, IDT_EDITCHECK_INTERVAL, nullptr);

	return true;
}

/*!
 * WM_DESTROYハンドラ
 *
 * WM_DESTROYはDestroyWindow関数によるウインドウ破棄中にポストされます。
 * このメッセージに戻り値はありません。
 *
 * @date 2006/07/09 ryoji 新規作成
 */
void CControlTray::OnDestroy(HWND hWnd)
{
	if (!GetTrayHwnd()) {
		return;	// 既に破棄されている
	}

	// ワーカーがまだ動いているなら、終了指示を出す
	if (m_Worker.joinable()) {
		m_Worker.request_stop();
		m_QueueCv.notify_one();
	}

	// ホットキーの破棄
	::UnregisterHotKey(hWnd, ID_HOTKEY_TRAYMENU);

	if (m_bCreatedTrayIcon) {	/* トレイにアイコンを作った */
		window::SendTrayMessage(hWnd, 0, NIM_DELETE, nullptr, std::nullopt);
	}

	// 「タスクトレイに常駐しない」設定でエディタ画面（Normal Process）を立ち上げたまま
	// セッション終了するような場合でも共有データ保存が行われなかったり中断されることが
	// 無いよう、ここでウィンドウが破棄される前に保存する
	//

	CDialog dlgExiting;

	if (m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog) {	/* 終了ダイアログを表示する */
		dlgExiting.DoModeless(
			m_hInstance,
			hWnd,
			IDD_EXITING,
			0L,
			SW_SHOW
		);
	}

	// スコープを抜けるとき閉じられるようにする
	using WindowHolder = cxx::ResourceHolder<&::DestroyWindow>;
	WindowHolder hWndExitingDlg{ dlgExiting.GetHwnd() };

	m_pShareData->m_sHandles.m_hwndTray = nullptr;

	/* 共有データの保存 */
	CShareData_IO::SaveShareData();

	hWndExitingDlg = nullptr;

	m_pShareData->m_sHandles.m_hwndTray = nullptr;

	Base::OnDestroy(hWnd);

	// Windows にスレッドの終了を要求します。
	::PostQuitMessage(0);
}

/*!
 * WM_CLOSEハンドラ
 *
 * ウインドウクローズが要求されたときに呼ばれる
 * このメッセージに戻り値はありません。
 */
void CControlTray::OnClose(HWND hWnd) const
{
	//すべてのウィンドウを閉じる
	if (!CloseAllEditor(FALSE, hWnd, TRUE, 0)) {
		return;
	}

	//ウィンドウを破棄する(DefWindowProcと同じだが、あえて書いておく)
	::DestroyWindow(hWnd);
}

/*!
 * WM_QUERYENDSESSIONハンドラ
 *
 * WM_QUERYENDSESSIONはシステム終了が要求されたときにポストされます。
 *
 * @note windowsx.h の定義が微妙なので独自に定義
 *
 * @retval true  システム終了を続行する
 * @retval false システム終了を中止する
 * 
 * @date 2000/01/31 genta Windows終了時の後処理
 */
bool CControlTray::OnQueryEndSession(HWND hWnd, UINT endSessionFlags) const
{
	UNREFERENCED_PARAMETER(endSessionFlags);

	// ここの実装は要改修
	//
	// 5秒以内に終わらない処理を走らせるなら、ShutdownBlockReasonCreateを呼ぶ必要があります。

	//すべてのウィンドウを閉じる
	if (!CloseAllEditor(FALSE, hWnd, TRUE, 0)) {
		return false;
	}

	return true;
}

/*!
 * WM_ENDSESSIONハンドラ
 *
 * WM_ENDSESSIONはWM_QUERYENDSESSIONメッセージの処理後にポストされます。
 * このメッセージに戻り値はありません。
 * 
 * @note windowsx.h の定義が微妙なので独自に定義
 *
 * @date 2000/01/31 genta Windows終了時の後処理
 */
void CControlTray::OnEndSession(HWND hWnd, bool bEndSession, UINT endSessionFlags)
{
	UNREFERENCED_PARAMETER(endSessionFlags);

	//	Windows終了時はWM_CLOSEが呼ばれない上，DestroyWindowを
	//	呼び出す必要もない．また，メッセージループに戻らないので
	//	メッセージループの後ろの処理をここで完了させる必要がある．

	//	もしWindowsの終了が中断されたのなら何もしない
	if (bEndSession)
		OnDestroy(hWnd);	// 2006.07.09 ryoji WM_DESTROY と同じ処理をする（トレイアイコンの破棄などもNT系では必要）
}

/*!
 * WM_TIMERハンドラ
 *
 * このメッセージの戻り値は0固定です。
 */
void CControlTray::OnTimer(HWND hWnd, UINT id)
{
	// 編集ウィンドウ存在確認。消えたウィンドウを抹消する
	if (IDT_EDITCHECK == id) {
		std::span nodes(m_pShareData->m_sNodes.m_pEditArr, std::min<size_t>(m_pShareData->m_sNodes.m_nEditArrNum, std::size(m_pShareData->m_sNodes.m_pEditArr)));
		if (const auto found = std::ranges::find_if(nodes, [](const auto& node) { return !IsSakuraMainWindow(node.GetHwnd()); }); found != nodes.end()) {
			found->GetGroup().DeleteEditWndList(found->GetHwnd());
			if (0 == m_pShareData->m_sNodes.m_nEditArrNum) {
				PostMessageW(hWnd, MYWM_DELETE_ME, 0, 0);
			}
		}
		return;
	}
}

/*!
 * WM_HOTKEYハンドラ
 *
 * このメッセージの戻り値は0固定です。
 */
void CControlTray::OnHotKey(HWND hWnd, int idHotKey, UINT fuModifiers, UINT vk) const
{
	// タスクトレイ左クリックメニューへのショートカットキー
	if (ID_HOTKEY_TRAYMENU != idHotKey
		|| wHotKeyMods != fuModifiers
		|| wHotKeyCode != vk
	)
	{
		return;
	}

	const auto hWndForeground = ::GetForegroundWindow();
	StaticString<100> szClassName;
	::GetClassNameW(hWndForeground, szClassName, int(std::size(szClassName)) - 1);

	StaticString<100> text;
	::GetWindowTextW(hWndForeground, std::data(text), int(std::size(text)));
	if (text == LS(STR_PROPCOMMON)) {
		return;
	}

	::PostMessageW(hWnd, MYWM_NOTIFYICON, 0, WM_LBUTTONUP);
}

bool CControlTray::OnSetTypeSetting(size_t index)
{
	if (m_pShareData->m_nTypesCount <= 0 || m_pShareData->m_nTypesCount <= index) {
		return false;
	}

	const auto& type = m_pShareData->m_sWorkBuffer.m_TypeConfig;
	if (0 == index) {
		m_pShareData->m_TypeBasis = type;
		m_pShareData->m_TypeBasis.m_nIdx = 0;
	}

	auto& types = CShareData::getInstance()->GetTypeSettings();
	*types[index] = type;
	types[index]->m_nIdx = int(index);

	auto& typeMini = m_pShareData->m_TypeMini[index];
	::wcscpy_s(typeMini.m_szTypeName, type.m_szTypeName);
	::wcscpy_s(typeMini.m_szTypeExts, type.m_szTypeExts);
	typeMini.m_id = type.m_id;
	typeMini.m_encoding = type.m_encoding;

	return true;
}

bool CControlTray::OnGetTypeSetting(size_t index)
{
	if (m_pShareData->m_nTypesCount <= 0 || m_pShareData->m_nTypesCount <= index) {
		return false;
	}

	m_pShareData->m_sWorkBuffer.m_TypeConfig = *(CShareData::getInstance()->GetTypeSettings()[index]);

	return true;
}

bool CControlTray::OnAddTypeSetting(size_t index)
{
	if (m_pShareData->m_nTypesCount < 0 || int(MAX_TYPES) <= m_pShareData->m_nTypesCount || m_pShareData->m_nTypesCount < index) {
		return false;
	}

	// 0:"共通" の前には入れない
	if (0 == index) {
		return false;
	}

	const auto nInsert = (int)index;
	auto& types = CShareData::getInstance()->GetTypeSettings();
	auto type = std::make_unique<STypeConfig>(*types[0]);	// 基本をコピー
	type->m_id = (::GetTickCount64() & 0x3fffffff) + nInsert * 0x10000;

	// 同じ名前のものがあったらその次にする
	auto nAddNameNum = nInsert + 1;
	::swprintf_s(type->m_szTypeName, LS(STR_TRAY_TYPE_NAME), nAddNameNum);
	for (auto k = 1; k < m_pShareData->m_nTypesCount; ++k) {
		if (0 == wcscmp(types[k]->m_szTypeName, type->m_szTypeName)) {
			nAddNameNum++;
			::swprintf_s(type->m_szTypeName, LS(STR_TRAY_TYPE_NAME), nAddNameNum);
			k = 0;
		}
	}
	type->m_szTypeExts[0] = L'\0';
	type->m_nRegexKeyMagicNumber = CRegexKeyword::GetNewMagicNumber();

	types.emplace(types.cbegin() + nInsert, std::move(type));

	//追加したタイプ別設定を取得する(typeはもう使えない)
	const auto& added = types[nInsert];

	auto& typesMini = m_pShareData->m_TypeMini;
	for (int i = m_pShareData->m_nTypesCount; nInsert < i; --i) {
		types[i]->m_nIdx = i;
		std::swap(typesMini[i - 1], typesMini[i]);
	}

	auto& typeMini = typesMini[nInsert];
	::wcscpy_s(typeMini.m_szTypeName, added->m_szTypeName);
	::wcscpy_s(typeMini.m_szTypeExts, added->m_szTypeExts);
	typeMini.m_id = added->m_id;
	typeMini.m_encoding = added->m_encoding;

	++m_pShareData->m_nTypesCount;

	return true;
}

bool CControlTray::OnDelTypeSetting(size_t index)
{
	if (m_pShareData->m_nTypesCount <= 0 || m_pShareData->m_nTypesCount <= index) {
		return false;
	}

	const auto nDelPos = (int)index;
	if (nDelPos <= 0) {
		return false;
	}

	auto& types = CShareData::getInstance()->GetTypeSettings();
	types.erase(types.cbegin() + nDelPos);

	auto& typesMini = m_pShareData->m_TypeMini;
	for (int i = nDelPos; i < std::size(types) - 1; ++i) {
		types[i]->m_nIdx = i;
		std::swap(typesMini[i], typesMini[i + 1]);
	}

	auto& typeMini = typesMini[m_pShareData->m_nTypesCount];
	typeMini.m_szTypeName[0] = L'\0';
	typeMini.m_szTypeExts[0] = L'\0';
	typeMini.m_id = 0;

	m_pShareData->m_nTypesCount--;

	return true;
}

/*!
	@brief 新規ウィンドウを作成する

	@author genta
	@date 2003.05.30 新規作成
	@date 2013.03.21 novice MRUは使用しない
*/
void CControlTray::OnNewEditor( bool bNewWindow )
{
	// 新規ウィンドウで開くオプションは、タブバー＆グループ化を前提とする
	bNewWindow = bNewWindow
				 && m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd != FALSE
				 && m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin == FALSE;

	// 編集ウインドウを開く
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = L"";
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	std::wstring strCurDir = CSakuraEnvironment::GetDlgInitialDir(true);
	OpenNewEditor( m_hInstance, GetTrayHwnd(), sLoadInfo, nullptr, false, strCurDir.c_str(), bNewWindow );
}

/*!
	新規編集ウィンドウの追加 ver 0

	@date 2000.10.24 genta WinExec -> CreateProcess．同期機能を付加
	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2003.05.30 genta 外部プロセス起動時のカレントディレクトリ指定を可能に．
	@date 2007.06.26 ryoji 新規編集ウィンドウは hWndParent と同じグループを指定して起動する
	@date 2008.04.19 ryoji MYWM_FIRST_IDLE 待ちを追加
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
bool CControlTray::OpenNewEditor(
	[[maybe_unused]] HINSTANCE			hInstance,			//!< [in] インスタンスID (実は未使用)
	HWND				hWndParent,			//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
	const SLoadInfo&	sLoadInfo,			//!< [in]
	const WCHAR*		szCmdLineOption,	//!< [in] 追加のコマンドラインオプション
	bool				sync,				//!< [in] trueなら新規エディタの起動まで待機する
	const WCHAR*		pszCurDir,			//!< [in] 新規エディタのカレントディレクトリ(NULL可)
	bool				bNewWindow			//!< [in] 新規エディタを新しいウインドウで開く
)
{
	/* 編集ウィンドウの上限チェック */
	if (const auto pShareData = GetDllShareDataPtr(); !pShareData || MAX_EDITWINDOWS <= pShareData->m_sNodes.m_nEditArrNum) {
		// L"編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"
		OkMessage(nullptr, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return false;
	}

	std::vector<std::wstring> args;

	// ファイル名
	if (!sLoadInfo.cFilePath.empty()) args.emplace_back(sLoadInfo.cFilePath.c_str());

	// コード指定
	if (IsValidCodeOrCPType(sLoadInfo.eCharCode)) args.emplace_back(std::format(L"-CODE={:d}", static_cast<int>(sLoadInfo.eCharCode)));

	// ビューモード指定
	if (sLoadInfo.bViewMode) args.emplace_back(L"-R");

	// 追加のコマンドラインオプション
	std::wstring_view cmdLineOptions{ szCmdLineOption ? szCmdLineOption : L"" };

	std::optional<std::filesystem::path> optWorkingDir = std::nullopt;
	if (pszCurDir && *pszCurDir) {
		optWorkingDir = pszCurDir;
	}

	// -- -- -- -- プロセス生成 -- -- -- -- //

	bool bRet = false;
	try {
		auto ep = CProcess::CreateEditorProcess(args, cmdLineOptions, optWorkingDir, std::nullopt, bNewWindow, sync);
		bRet = ep.hWnd;
	}
	catch (const std::system_error& e) {
		// L"'%s'\nプロセスの起動に失敗しました。\n%s"
		ErrorMessage(hWndParent, LS(STR_TRAY_CREATEPROC1),
			GetExeFileName().c_str(),
			to_wchar(e.what())
		);
	}

	return bRet;
}

/*!	新規編集ウィンドウの追加 ver 2:

	@date Oct. 24, 2000 genta create.
	@date Feb. 25, 2012 novice -CODE/-RはOpenNewEditor側で処理するので削除
*/
bool CControlTray::OpenNewEditor2(
	HINSTANCE		hInstance,
	HWND			hWndParent,
	const EditInfo*	pfi,
	bool			bViewMode,
	bool			sync,
	bool			bNewWindow			//!< [in] 新規エディタを新しいウインドウで開く
)
{
	/* 編集ウィンドウの上限チェック */
	if (const auto pShareData = GetDllShareDataPtr(); !pShareData || MAX_EDITWINDOWS <= pShareData->m_sNodes.m_nEditArrNum) {
		// L"編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"
		OkMessage(nullptr, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return false;
	}

	// 追加のコマンドラインオプション
	CCommandLineString cCmdLine;
	if( pfi != nullptr ){
		if( pfi->m_ptCursor.x >= 0					)cCmdLine.AppendF( L" -X=%d", int(pfi->m_ptCursor.x) + 1 );
		if( pfi->m_ptCursor.y >= 0					)cCmdLine.AppendF( L" -Y=%d", int(pfi->m_ptCursor.y) + 1 );
		if( pfi->m_nViewLeftCol >= CLayoutInt(0)	)cCmdLine.AppendF( L" -VX=%d", (Int)pfi->m_nViewLeftCol + 1 );
		if( pfi->m_nViewTopLine >= CLayoutInt(0)	)cCmdLine.AppendF( L" -VY=%d", (Int)pfi->m_nViewTopLine + 1 );
	}
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = pfi ? pfi->m_szPath : L"";
	sLoadInfo.eCharCode = pfi ? pfi->m_nCharCode : CODE_NONE;
	sLoadInfo.bViewMode = bViewMode;
	return OpenNewEditor( hInstance, hWndParent, sLoadInfo, cCmdLine.c_str(), sync, nullptr, bNewWindow );
}
//	To Here Oct. 24, 2000 genta

void CControlTray::ActiveNextWindow(HWND hwndParent)
{
	/* 現在開いている編集窓のリストを得る */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* 自分のウィンドウを調べる */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( hwndParent == pEditNodeArr[i].GetHwnd() )
			{
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// 前のウィンドウ
			int		j;
			for( j = i - 1; j >= 0; --j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j < 0 ){
				for( j = nRowNum - 1; j > i; --j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* 前のウィンドウをアクティブにする */
			HWND	hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow( hwndWork );
			/* 最後のペインをアクティブにする */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
		}
		delete [] pEditNodeArr;
	}
}

void CControlTray::ActivePrevWindow(HWND hwndParent)
{
	/* 現在開いている編集窓のリストを得る */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* 自分のウィンドウを調べる */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( hwndParent == pEditNodeArr[i].GetHwnd() ){
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// 次のウィンドウ
			int		j;
			for( j = i + 1; j < nRowNum; ++j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j >= nRowNum ){
				for( j = 0; j < i; ++j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* 次のウィンドウをアクティブにする */
			HWND	hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow( hwndWork );
			/* 最初のペインをアクティブにする */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
		}
		delete [] pEditNodeArr;
	}
}

/*!	サクラエディタの全終了

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2006.12.25 ryoji 複数の編集ウィンドウを閉じるときの確認（引数追加）
*/
void CControlTray::TerminateApplication(
	HWND hWndFrom	//!< [in] 呼び出し元のウィンドウハンドル
)
{
	DLLSHAREDATA* pShareData = &GetDllShareData();	/* 共有データ構造体のアドレスを返す */

	/* 現在の編集ウィンドウの数を調べる */
	if( pShareData->m_Common.m_sGeneral.m_bExitConfirm ){	//終了時の確認
		if( 0 < CAppNodeGroupHandle(0).GetEditorWindowsNum() ){
			if( IDYES != ::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				LS(STR_TRAY_EXITALL)
			) ){
				return;
			}
		}
	}
	/* 「すべてのウィンドウを閉じる」要求 */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	BOOL bCheckConfirm = (pShareData->m_Common.m_sGeneral.m_bExitConfirm)? FALSE: TRUE;	// 2006.12.25 ryoji 終了確認済みならそれ以上は確認しない
	if( CloseAllEditor( bCheckConfirm, hWndFrom, TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
		::PostMessageAny( pShareData->m_sHandles.m_hwndTray, WM_CLOSE, 0, 0 );
	}
	return;
}

/*!	すべてのウィンドウを閉じる

	@date Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2006.12.25 ryoji 複数の編集ウィンドウを閉じるときの確認（引数追加）
	@date 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加
	@date 2007.06.20 ryoji nGroup引数を追加
*/
BOOL CControlTray::CloseAllEditor(
	BOOL	bCheckConfirm,	//!< [in] [すべて閉じる]確認オプションに従って問い合わせをするかどうか
	HWND	hWndFrom,		//!< [in] 呼び出し元のウィンドウハンドル
	BOOL	bExit,			//!< [in] TRUE: 編集の全終了 / FALSE: すべて閉じる
	int		nGroup			//!< [in] グループID
)
{
	EditNode*	pWndArr;
	int		n;

	n = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}
	
	/* 全編集ウィンドウへ終了要求を出す */
	BOOL	bRes = CAppNodeGroupHandle(nGroup).RequestCloseEditor( pWndArr, n, bExit, bCheckConfirm, hWndFrom );	// 2007.02.13 ryoji bExitを引き継ぐ
	delete []pWndArr;
	return bRes;
}

/*! ポップアップメニュー(トレイ左ボタン) */
EFunctionCode CControlTray::TrackPopupMenu_L(HWND hWnd)
{
	int			i;
	int			j;
	HMENU		hMenuTop;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	WCHAR		szMenu[100 + MAX_PATH * 2];	//	Jan. 19, 2001 genta
	EditInfo*	pfi;

	if (const auto dwRet = ::WaitForSingleObject(m_hQueue, 0); WAIT_TIMEOUT == dwRet) return F_0;

	m_cMenuDrawer.ResetContents();
	CFileNameManager::getInstance()->TransformFileName_MakeCache();

	// リソースを使わないように
	hMenuTop = ::CreatePopupMenu();
	hMenu = ::CreatePopupMenu();
	m_cMenuDrawer.MyAppendMenu( hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenu, L"TrayL", L"" );

	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, L"", L"N", FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, L"", L"O", FALSE );

	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG, L"", L"G", FALSE );
	m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );

	/* MRUリストのファイルのリストをメニューにする */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	const CMRUFile cMRU;
	hMenuPopUp = cMRU.CreateMenu( &m_cMenuDrawer );	//	ファイルメニュー
	int nEnable = (cMRU.MenuLength() > 0 ? 0 : MF_GRAYED);
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | nEnable, (UINT_PTR)hMenuPopUp , LS( F_FILE_RCNTFILE_SUBMENU ), L"F" );

	/* 最近使ったフォルダーのメニューを作成 */
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
	const CMRUFolder cMRUFolder;
	hMenuPopUp = cMRUFolder.CreateMenu( &m_cMenuDrawer );
	nEnable = (cMRUFolder.MenuLength() > 0 ? 0 : MF_GRAYED);
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP| nEnable, (UINT_PTR)hMenuPopUp, LS( F_FILE_RCNTFLDR_SUBMENU ), L"D" );

	/* 履歴の管理のメニューを作成 */
	m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FAVORITE, L"", L"M", FALSE );

	m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL, L"", L"Z", FALSE );	// Jan. 24, 2005 genta

	/* 現在開いている編集窓のリストをメニューにする */
	j = 0;
	for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
		if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].GetHwnd() ) ){
			++j;
		}
	}

	if( j > 0 ){
		m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );

		NONCLIENTMETRICS met;
		met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, met.cbSize);
		CDCFont dcFont(met.lfMenuFont);

		j = 0;
		for( i = 0; i < m_pShareData->m_sNodes.m_nEditArrNum; ++i ){
			if( IsSakuraMainWindow( m_pShareData->m_sNodes.m_pEditArr[i].GetHwnd() ) ){
				/* トレイからエディタへの編集ファイル名要求通知 */
				::SendMessage( m_pShareData->m_sNodes.m_pEditArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0 );
				pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

				// メニューラベル。1からアクセスキーを振る
				CFileNameManager::getInstance()->GetMenuFullLabel_WinList( szMenu, int(std::size(szMenu)), pfi, m_pShareData->m_sNodes.m_pEditArr[i].m_nId, i, dcFont.GetHDC() );
				m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMenu, L"", FALSE );
				++j;
			}
		}
	}
	m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, L"", L"Q", FALSE );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO アクセスキー変更(L→Q)	// 2006.10.21 ryoji 表示文字列変更	// 2007.02.13 ryoji →F_EXITALLEDITORS
	if( j == 0 ){
		::EnableMenuItem( hMenu, F_EXITALLEDITORS, MF_BYCOMMAND | MF_GRAYED );	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	// 2007.02.13 ryoji →F_EXITALLEDITORS
		::EnableMenuItem( hMenu, F_FILESAVEALL, MF_BYCOMMAND | MF_GRAYED );	// Jan. 24, 2005 genta
	}

	//	Jun. 9, 2001 genta ソフトウェア名改称
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, L"", L"X", FALSE );	//Dec. 26, 2000 JEPRO F_に変更

	MenuHolder menuHolder{ hMenuTop };

	const auto eFuncCode = window::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_LEFTBUTTON
		,
		hWnd
	);

	menuHolder = nullptr;

	return eFuncCode;
}

//キーワード：トレイ右クリックメニュー順序
//	Oct. 12, 2000 JEPRO ポップアップメニュー(トレイ左ボタン) を参考にして新たに追加した部分

/*! ポップアップメニュー(トレイ右ボタン) */
EFunctionCode CControlTray::TrackPopupMenu_R(HWND hWnd)
{
	HMENU	hMenuTop;
	HMENU	hMenu;

	if (const auto dwRet = ::WaitForSingleObject(m_hQueue, 0); WAIT_TIMEOUT == dwRet) return F_0;

	m_cMenuDrawer.ResetContents();

	// リソースを使わないように
	hMenuTop = ::CreatePopupMenu();
	hMenu = ::CreatePopupMenu();
	m_cMenuDrawer.MyAppendMenu( hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenu, L"TrayR", L"" );

	/* トレイ右クリックの「ヘルプ」メニュー */
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , L"", L"O", FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , L"", L"S", FALSE );	//Nov. 25, 2000 JEPRO 「トピックの」→「キーワード」に変更
	m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST, L"", L"L", FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_OPTION, L"", L"C", FALSE );
	m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT, L"", L"A", FALSE );	//Dec. 25, 2000 JEPRO F_に変更
	m_cMenuDrawer.MyAppendMenuSep( hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr, FALSE );
	//	Jun. 18, 2001 genta ソフトウェア名改称
	m_cMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, L"", L"X", FALSE );

	MenuHolder menuHolder{ hMenuTop };

	const auto eFuncCode = window::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_LEFTBUTTON
		,
		hWnd
	);

	menuHolder = nullptr;

	return eFuncCode;
}
