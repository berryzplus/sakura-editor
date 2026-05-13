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
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CNormalProcess.h"

#include "_main/CControlProcess.h"	//cxx::MutexHolder

#include "CCommandLine.h"
#include "CControlTray.h"
#include "window/CEditWnd.h" // 2002/2/3 aroka
#include "agent/CGrepAgent.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h" // 2003/03/28 MIK
#include "debug/CRunningTimer.h"
#include "util/window.h"
#include "util/file.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "CAppMode.h"
#include "apiwrap/DarkMode.h"
#include "env/CDocTypeManager.h"
#include "apiwrap/StdApi.h"
#include "CSelectLang.h"
#include "env/CShareData.h"
#include "config/system_constants.h"

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
bool CNormalProcess::InitializeProcess(int nCmdShow)
{
	MY_RUNNINGTIMER( cRunningTimer, L"NormalProcess::Init" );

	if (!*m_OleInit) return false;

	const auto pszProfileName = GetProfileName();

	// 初期化ミューテックスの名前を組み立てる
	std::wstring strMutexInitName = GSTR_MUTEX_SAKURA_INIT;
	strMutexInitName += pszProfileName;

	/* プロセス初期化の目印 */
	cxx::MutexHolder hMutex{ ::CreateMutexW(nullptr, TRUE, strMutexInitName.c_str()) };
	if (!hMutex) {
		//ErrorBeep();
		//TopErrorMessage( nullptr, L"CreateMutex()失敗。\n終了します。" );
		return false;
	}

	if (!hMutex.try_lock_for(std::chrono::seconds(15))){// 別の誰かが起動中
		//TopErrorMessage( nullptr, L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。" );
		return false;
	}

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += pszProfileName;

	// トレイウインドウを検索する
	if (!cxx::FindWindowW(trayWndClassName, trayWndClassName)) {
		// コントロールプロセスを起動する
		CProcess::CreateControlProcess(pszProfileName);
	}

	/* 共有メモリを初期化する */
	if (!GetShareData().InitShareData()) {
		return false;
	}

	/* ダークモード設定を反映する */
	ApplyDarkModeSetting(GetDllShareData().m_Common.m_sWindow.m_bDarkMode);

	/* 言語を選択する */
	CSelectLang::ChangeLang( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );

	/* コマンドラインオプション */
	const auto& fi = CCommandLine::getInstance()->GetEditInfoRef(); // 2002/2/8 aroka ここに移動

	//複数ファイル読み込み
	auto files = CCommandLine::getInstance()->GetFiles();
	if (auto fileNum = std::ssize(files); 0 < fileNum) {
		// ファイルドロップ数の上限に合わせる
		if (const auto nDropFileNumMax = GetDllShareData().m_Common.m_sFile.m_nDropFileNumMax - 1; nDropFileNumMax < fileNum) {
			files = files.subspan(0, nDropFileNumMax);
		}

		const auto bViewMode = CCommandLine::getInstance()->IsViewMode();

		EditInfo ei{ fi };

		CControlTray::OpenNewEditor2(GetProcessInstance(), HWND(nullptr), &ei, bViewMode);

		for (const auto& file: files) {
			if (STRUNCATE == ::wcscpy_s(ei.m_szPath, file.c_str()) || !CControlTray::OpenNewEditor2(GetProcessInstance(), HWND(nullptr), &ei, bViewMode)) {
				break;
			}
		}

		return false;
	}

	// ファイルが既に開かれていたらアクティブにして抜ける
	if (HWND hwndOwner; fi.m_szPath[0] != L'\0' && GetShareData().ActiveAlreadyOpenedWindow(fi.m_szPath, &hwndOwner, fi.m_nCharCode)) {
		//カーソル位置が引数に指定されていたら指定位置にジャンプ
		if (0 <= fi.m_ptCursor.y) {	//	行の指定があるか
			//行桁情報の連携用バッファを参照する
			auto& pt = GetDllShareData().m_sWorkBuffer.m_LogicPoint;

			//桁指定をバッファに反映する
			pt.x = fi.m_ptCursor.x;

			//桁の指定が無い場合、エディタから取得する
			if (fi.m_ptCursor.x < 0) {
				::SendMessageTimeoutW(hwndOwner, MYWM_GETCARETPOS, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 0, nullptr);
			}

			//行指定をバッファに反映する
			pt.y = fi.m_ptCursor.y;

			//行桁指定をエディタに反映する
			::SendMessageTimeoutW(hwndOwner, MYWM_SETCARETPOS, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 0, nullptr);
		}

		//エディタをアクティブにする
		ActivateFrameWindow(hwndOwner);

		return false;
	}

	hMutex = nullptr;

	//ドキュメントを作成
	m_pcEditDoc = std::make_unique<CEditDoc>();

	//メインウィンドウオブジェクトのインスタンスを作成
	m_pcEditWnd = std::make_unique<CEditWnd>();

	// エディタアプリケーションを作成。2007.10.23 kobake
	m_pcEditApp = std::make_unique<CEditApp>();

	// プラグイン読み込み
	MY_TRACETIME( cRunningTimer, L"Before Init Jack" );
	/* ジャック初期化 */
	CJackManager::getInstance();
	MY_TRACETIME( cRunningTimer, L"After Init Jack" );

	MY_TRACETIME( cRunningTimer, L"Before Load Plugins" );
	/* プラグイン読み込み */
	CPluginManager::getInstance()->LoadAllPlugin();
	MY_TRACETIME( cRunningTimer, L"After Load Plugins" );

	auto bGrepDlg = CCommandLine::getInstance()->IsGrepDlg();
	auto bGrepMode = CCommandLine::getInstance()->IsGrepMode() || bGrepDlg;

	const auto gi = CCommandLine::getInstance()->GetGrepInfoRef();

	auto& cDlgGrep = gi.bGrepReplace
		? GetEditWnd().m_cDlgGrepReplace
		: GetEditWnd().m_cDlgGrep;

	if (bGrepMode) {
		cDlgGrep.m_strText = gi.cmGrepKey.GetStringPtr();
		cDlgGrep.m_bSetText = true;
		cDlgGrep.m_szFile = gi.cmGrepFile.GetStringPtr();
		cDlgGrep.m_szFolder = gi.cmGrepFolder.GetStringPtr();

		GetDllShareData().m_Common.m_sSearch.m_bGrepSubFolder = gi.bGrepSubFolder;
		GetDllShareData().m_Common.m_sSearch.m_sSearchOption = gi.sGrepSearchOption;
		GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet = gi.nGrepCharSet;
		GetDllShareData().m_Common.m_sSearch.m_nGrepOutputLineType = gi.nGrepOutputLineType;
		GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle = gi.nGrepOutputStyle;
		GetDllShareData().m_Common.m_sSearch.m_bGrepOutputFileOnly = gi.bGrepOutputFileOnly;
		GetDllShareData().m_Common.m_sSearch.m_bGrepOutputBaseFolder = gi.bGrepOutputBaseFolder;
		GetDllShareData().m_Common.m_sSearch.m_bGrepSeparateFolder = gi.bGrepSeparateFolder;

		if (auto pDlgGrepRep = dynamic_cast<CDlgGrepReplace*>(&cDlgGrep)) {
			pDlgGrepRep->m_strText2 = gi.cmGrepRep.GetStringPtr();
			pDlgGrepRep->m_bPaste = gi.bGrepPaste;
			pDlgGrepRep->m_bBackup = gi.bGrepBackup;
		}

		bGrepDlg = cDlgGrep.GetData() <= 0;
	}

	//-GREPDLGでダイアログを出す。　引数も反映（2002/03/24 YAZAKI）
	if (bGrepDlg) {
		if (cDlgGrep.DoModal(GetProcessInstance(), HWND(nullptr), nullptr)) {
			CControlTray::DoGrepCreateWindow(GetProcessInstance(), GetDllShareData().m_sHandles.m_hwndTray, cDlgGrep);

			return false;

		} else {
			bGrepMode = false;
		}
	}

	// グループIDを取得
	int nGroupId = CCommandLine::getInstance()->GetGroupId();
	if( GetDllShareData().m_Common.m_sTabBar.m_bNewWindow && nGroupId == -1 ){
		nGroupId = CAppNodeManager::getInstance()->GetFreeGroupId();
	}

	// メインウインドウを作成
	m_pcEditApp->Create(GetProcessInstance(), nGroupId);
	auto pEditWnd = GetEditWndPtr();
	auto hEditWnd = pEditWnd->GetHwnd();
	if (!hEditWnd) {
		return false;	// 2009.06.23 ryoji CEditWnd::Create()失敗のため終了
	}

	SetMainWindow(hEditWnd);

	/* コマンドラインの解析 */	 // 2002/2/8 aroka ここに移動
	const auto bDebugMode = CCommandLine::getInstance()->IsDebugMode();

	MY_TRACETIME( cRunningTimer, L"CheckFile" );

	// -1: SetDocumentTypeWhenCreate での強制指定なし
	const CTypeConfig nType = (fi.m_szDocType[0] == '\0' ? CTypeConfig(-1) : CDocTypeManager().GetDocumentTypeOfExt(fi.m_szDocType));

	if( bDebugMode ){
		/* デバッグモニタモードに設定 */
		pEditWnd->GetDocument()->SetCurDirNotitle();
		CAppMode::getInstance()->SetDebugModeON();
		if( !CAppMode::getInstance()->IsDebugMode() ){
			// デバッグではなくて(無題)
			CAppNodeManager::getInstance()->GetNoNameNumber(hEditWnd);
			pEditWnd->UpdateCaption();
		}
		// 2004.09.20 naoh アウトプット用タイプ別設定
		// 文字コードを有効とする Uchi 2008/6/8
		// 2010.06.16 Moca アウトプットは CCommnadLineで -TYPE=output 扱いとする
		pEditWnd->SetDocumentTypeWhenCreate( fi.m_nCharCode, false, nType );
		pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを表示する
	}
	else if (bGrepMode) {
		// 2010.06.16 Moca Grepでもオプション指定を適用
		pEditWnd->SetDocumentTypeWhenCreate( fi.m_nCharCode, false, nType );
		pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを予め表示しておく

		GetGrepAgent()->DoGrep(
			&pEditWnd->GetActiveView(),
			cDlgGrep,
			gi.bGrepHeader,
			gi.bGrepStdout,
			gi.bGrepCurFolder
		);

		pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを再解析する
	}
	else{
		// 2004.05.13 Moca さらにif分の中から前に移動
		// ファイル名が与えられなくてもReadOnly指定を有効にするため．
		const auto bViewMode = CCommandLine::getInstance()->IsViewMode(); // 2002/2/8 aroka ここに移動
		if( fi.m_szPath[0] != L'\0' ){
			//	Mar. 9, 2002 genta 文書タイプ指定
			pEditWnd->OpenDocumentWhenStart(
				SLoadInfo(
					fi.m_szPath,
					fi.m_nCharCode,
					bViewMode,
					nType
				)
			);
			// 読み込み中断して「(無題)」になった時（他プロセスからのロックなど）もオプション指定を有効にする
			// Note. fi.m_nCharCode で文字コードが明示指定されていても、読み込み中断しない場合は別の文字コードが選択されることがある。
			//       以前は「(無題)」にならない場合でも無条件に SetDocumentTypeWhenCreate() を呼んでいたが、
			//       「前回と異なる文字コード」の問い合わせで前回の文字コードが選択された場合におかしくなっていた。
			if( !pEditWnd->GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
				// 読み込み中断して「(無題)」になった
				// ---> 無効になったオプション指定を有効にする
				pEditWnd->SetDocumentTypeWhenCreate(
					fi.m_nCharCode,
					bViewMode,
					nType
				);
			}
			//	Nov. 6, 2000 genta
			//	キャレット位置の復元のため
			//	オプション指定がないときは画面移動を行わないようにする
			//	Oct. 19, 2001 genta
			//	未設定＝-1になるようにしたので，安全のため両者が指定されたときだけ
			//	移動するようにする． || → &&
			if( ( CLayoutInt(0) <= fi.m_nViewTopLine && CLayoutInt(0) <= fi.m_nViewLeftCol )
				&& fi.m_nViewTopLine < pEditWnd->GetDocument()->m_cLayoutMgr.GetLineCount() ){
				pEditWnd->GetActiveView().GetTextArea().SetViewTopLine( fi.m_nViewTopLine );
				pEditWnd->GetActiveView().GetTextArea().SetViewLeftCol( fi.m_nViewLeftCol );
			}

			//	オプション指定がないときはカーソル位置設定を行わないようにする
			//	Oct. 19, 2001 genta
			//	0も位置としては有効な値なので判定に含めなくてはならない
			if( 0 <= fi.m_ptCursor.x || 0 <= fi.m_ptCursor.y ){
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				CLayoutPoint ptPos;
				pEditWnd->GetDocument()->m_cLayoutMgr.LogicToLayout(
					fi.m_ptCursor,
					&ptPos
				);

				// From Here Mar. 28, 2003 MIK
				// 改行の真ん中にカーソルが来ないように。
				// 2008.08.20 ryoji 改行単位の行番号を渡すように修正
				const CDocLine *pTmpDocLine = pEditWnd->GetDocument()->m_cDocLineMgr.GetLine( fi.m_ptCursor.GetY2() );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_ptCursor.x ) ptPos.x--;
				}
				// To Here Mar. 28, 2003 MIK

				pEditWnd->GetActiveView().GetCaret().MoveCursor( ptPos, true );
				pEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev =
					pEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();
			}
			pEditWnd->GetActiveView().RedrawAll();
		}
		else{
			pEditWnd->GetDocument()->SetCurDirNotitle();	// (無題)ウィンドウ
			// 2004.05.13 Moca ファイル名が与えられなくてもReadOnlyとタイプ指定を有効にする
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.m_nCharCode,
				bViewMode,	// ビューモードか
				nType
			);
		}
		if( !pEditWnd->GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
			pEditWnd->GetDocument()->SetCurDirNotitle();	// (無題)ウィンドウ
			CAppNodeManager::getInstance()->GetNoNameNumber(hEditWnd);
			pEditWnd->UpdateCaption();
		}
	}

	//	YAZAKI 2002/05/30 IMEウィンドウの位置がおかしいのを修正。
	pEditWnd->GetActiveView().SetIMECompFormPos();

	//WM_SIZEをポスト
	{	// ファイル読み込みしなかった場合にはこの WM_SIZE がアウトライン画面を配置する
		if( !::IsIconic( hEditWnd ) ){
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::PostMessageAny( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	//再描画
	::InvalidateRect(hEditWnd, nullptr, TRUE);
	::UpdateWindow(hEditWnd);

	//プラグイン：EditorStartイベント実行
	CJackManager::getInstance()->InvokePlugins(PP_EDITOR_START, &pEditWnd->GetActiveView());

	// 2006.09.03 ryoji オープン後自動実行マクロを実行する
	if( !( bDebugMode || bGrepMode ) )
		pEditWnd->GetDocument()->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );

	if (!bGrepMode) {
		// 起動時マクロオプション
		if (LPCWSTR pszMacro = CCommandLine::getInstance()->GetMacro(); pszMacro && pszMacro[0] != L'\0') {
			LPCWSTR pszMacroType = CCommandLine::getInstance()->GetMacroType();
			pEditWnd->GetActiveView().GetCommander().HandleCommand( F_EXECEXTMACRO, true, (LPARAM)pszMacro, (LPARAM)pszMacroType, 0, 0 );
		}
	}

	//プラグイン：DocumentOpenイベント実行
	CJackManager::getInstance()->InvokePlugins( PP_DOCUMENT_OPEN, &pEditWnd->GetActiveView() );

	if (bGrepMode && (!bGrepDlg && gi.bGrepStdout)) {
		return false;	// 即時終了
	}

	return hEditWnd;
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
