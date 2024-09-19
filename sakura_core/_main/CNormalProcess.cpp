﻿/*!	@file
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
#include "_main/CNormalProcess.h"

#include "apiwrap/kernel/handle_closer.hpp"

#include "CEditApp.h"

#include "_main/CControlTray.h"
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

CEditorProcess* getEditorProcess() noexcept
{
	if (auto process = dynamic_cast<CEditorProcess*>(CProcess::getInstance())) {
		return process;
	}
	return nullptr;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CNormalProcess::CNormalProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine, int nCmdShow) noexcept
	: CProcess(hInstance, std::move(pCommandLine), nCmdShow)
{
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

	const auto profileName = GetCCommandLine().GetProfileName();

	// ミューテックスを使って排他ロックをかける
	std::wstring mutexName = GSTR_MUTEX_SAKURA_INIT;
	if (profileName && *profileName)
	{
		mutexName += profileName;
	}
	auto hMutex = CreateMutexW(nullptr, true, mutexName);
	if (!hMutex)
	{
		throw process_init_failed( LS(STR_ERR_DLGNRMPROC1) ); // L"CreateMutex()失敗。\n終了します。"
	}

	// ミューテックスハンドルをスマートポインタに入れる
	handleHolder mutexHolder( hMutex, handle_closer() );

	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		if (const auto waitResult = WaitForSingleObject(hMutex, 15 * 1000);
			WAIT_FAILED == waitResult || WAIT_TIMEOUT == waitResult)
		{
			throw process_init_failed( LS(STR_ERR_DLGNRMPROC2) ); // L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"
		}
	}

	InitProcess();

	//ウィンドウの作成
	if (!GetEditWnd()->CreateMainWnd(GetCmdShow())) {
		return false;	// 2009.06.23 ryoji CEditWnd::Create()失敗のため終了
	}

	// ミューテックスハンドルをスマートポインタから切り離す
	mutexHolder.release();

	auto pEditWnd = GetEditWnd();

	/* コマンドラインの解析 */	 // 2002/2/8 aroka ここに移動
	const auto bDebugMode = GetCCommandLine().IsDebugMode();
	const auto bGrepMode  = GetCCommandLine().IsGrepMode();
	const auto bGrepDlg   = GetCCommandLine().IsGrepDlg();

	MY_TRACETIME( cRunningTimer, L"CheckFile" );

	EditInfo fi = {};
	GetCCommandLine().GetEditInfo(&fi); // 2002/2/8 aroka ここに移動

	// -1: SetDocumentTypeWhenCreate での強制指定なし
	const CTypeConfig nType = (fi.m_szDocType[0] == '\0' ? CTypeConfig(-1) : CDocTypeManager().GetDocumentTypeOfExt(fi.m_szDocType));

	GrepInfo gi = {};
	GetCCommandLine().GetGrepInfo(&gi);

	if( bDebugMode ){
		/* デバッグモニタモードに設定 */
		pEditWnd->GetDocument()->SetCurDirNotitle();
		CAppMode::getInstance()->SetDebugModeON();
		if( !CAppMode::getInstance()->IsDebugMode() ){
			// デバッグではなくて(無題)
			CAppNodeManager::getInstance()->GetNoNameNumber( pEditWnd->GetHwnd() );
			pEditWnd->UpdateCaption();
		}
		// 2004.09.20 naoh アウトプット用タイプ別設定
		// 文字コードを有効とする Uchi 2008/6/8
		// 2010.06.16 Moca アウトプットは CCommnadLineで -TYPE=output 扱いとする
		pEditWnd->SetDocumentTypeWhenCreate( fi.m_nCharCode, false, nType );
		pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを表示する
	}
	else if( bGrepMode ){
		/* GREP */
		// 2010.06.16 Moca Grepでもオプション指定を適用
		pEditWnd->SetDocumentTypeWhenCreate( fi.m_nCharCode, false, nType );
		pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを予め表示しておく
		HWND hEditWnd = pEditWnd->GetHwnd();
		if( !::IsIconic( hEditWnd ) && pEditWnd->m_cDlgFuncList.GetHwnd() ){
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessageAny( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
		if( !bGrepDlg ){
			// Grepでは対象パス解析に現在のカレントディレクトリを必要とする
			// pEditWnd->GetDocument()->SetCurDirNotitle();
			// 2003.06.23 Moca GREP実行前にMutexを解放
			//	こうしないとGrepが終わるまで新しいウィンドウを開けない
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			this->m_pcEditApp->m_pcGrepAgent->DoGrep(
				&pEditWnd->GetActiveView(),
				gi.bGrepReplace,
				&gi.cmGrepKey,
				&gi.cmGrepRep,
				&gi.cmGrepFile,
				&gi.cmGrepFolder,
				gi.bGrepCurFolder,
				gi.bGrepSubFolder,
				gi.bGrepStdout,
				gi.bGrepHeader,
				gi.sGrepSearchOption,
				gi.nGrepCharSet,	//	2002/09/21 Moca
				gi.nGrepOutputLineType,
				gi.nGrepOutputStyle,
				gi.bGrepOutputFileOnly,
				gi.bGrepOutputBaseFolder,
				gi.bGrepSeparateFolder,
				gi.bGrepPaste,
				gi.bGrepBackup
			);
			pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを再解析する
		}
		else{
			CAppNodeManager::getInstance()->GetNoNameNumber( pEditWnd->GetHwnd() );
			pEditWnd->UpdateCaption();
			
			//-GREPDLGでダイアログを出す。　引数も反映（2002/03/24 YAZAKI）
			if( gi.cmGrepKey.GetStringLength() < _MAX_PATH ){
				CSearchKeywordManager().AddToSearchKeyArr( gi.cmGrepKey.GetStringPtr() );
			}
			if( gi.cmGrepFile.GetStringLength() < MAX_GREP_PATH ){
				CSearchKeywordManager().AddToGrepFileArr( gi.cmGrepFile.GetStringPtr() );
			}
			CNativeW cmemGrepFolder = gi.cmGrepFolder;
			if( gi.cmGrepFolder.GetStringLength() < MAX_GREP_PATH ){
				CSearchKeywordManager().AddToGrepFolderArr( gi.cmGrepFolder.GetStringPtr() );
				// 2013.05.21 指定なしの場合はカレントフォルダーにする
				if( cmemGrepFolder.GetStringLength() == 0 ){
					WCHAR szCurDir[_MAX_PATH];
					::GetCurrentDirectory( _countof(szCurDir), szCurDir );
					cmemGrepFolder.SetString( szCurDir );
				}
			}
			GetDllShareData().m_Common.m_sSearch.m_bGrepSubFolder = gi.bGrepSubFolder;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption = gi.sGrepSearchOption;
			GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet = gi.nGrepCharSet;
			GetDllShareData().m_Common.m_sSearch.m_nGrepOutputLineType = gi.nGrepOutputLineType;
			GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle = gi.nGrepOutputStyle;
			// 2003.06.23 Moca GREPダイアログ表示前にMutexを解放
			//	こうしないとGrepが終わるまで新しいウィンドウを開けない
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			hMutex = NULL;
			
			//	Oct. 9, 2003 genta コマンドラインからGERPダイアログを表示させた場合に
			//	引数の設定がBOXに反映されない
			pEditWnd->m_cDlgGrep.m_strText = gi.cmGrepKey.GetStringPtr();		/* 検索文字列 */
			pEditWnd->m_cDlgGrep.m_bSetText = true;
			int nSize = _countof2(pEditWnd->m_cDlgGrep.m_szFile);
			wcsncpy( pEditWnd->m_cDlgGrep.m_szFile, gi.cmGrepFile.GetStringPtr(), nSize );	/* 検索ファイル */
			pEditWnd->m_cDlgGrep.m_szFile[nSize-1] = L'\0';
			nSize = _countof2(pEditWnd->m_cDlgGrep.m_szFolder);
			wcsncpy( pEditWnd->m_cDlgGrep.m_szFolder, cmemGrepFolder.GetStringPtr(), nSize );	/* 検索フォルダー */
			pEditWnd->m_cDlgGrep.m_szFolder[nSize-1] = L'\0';

			// Feb. 23, 2003 Moca Owner windowが正しく指定されていなかった
			int nRet = pEditWnd->m_cDlgGrep.DoModal( GetProcessInstance(), pEditWnd->GetHwnd(),  NULL);
			if( FALSE != nRet ){
				pEditWnd->GetActiveView().GetCommander().HandleCommand(F_GREP, true, 0, 0, 0, 0);
			}else{
				// 自分はGrepでない
				pEditWnd->GetDocument()->SetCurDirNotitle();
			}
			pEditWnd->m_cDlgFuncList.Refresh();	// アウトラインを再解析する
		}

		//プラグイン：EditorStartイベント実行
		CJackManager::getInstance()->InvokePlugins( PP_EDITOR_START, &pEditWnd->GetActiveView() );

		//プラグイン：DocumentOpenイベント実行
		CJackManager::getInstance()->InvokePlugins( PP_DOCUMENT_OPEN, &pEditWnd->GetActiveView() );

		if( !bGrepDlg && gi.bGrepStdout ){
			// 即時終了
			PostMessageCmd( pEditWnd->GetHwnd(), MYWM_CLOSE, PM_CLOSE_GREPNOCONFIRM | PM_CLOSE_EXIT, (LPARAM)NULL );
		}

		return true; // 2003.06.23 Moca
	}
	else{
		// 2004.05.13 Moca さらにif分の中から前に移動
		// ファイル名が与えられなくてもReadOnly指定を有効にするため．
		const auto bViewMode = GetCCommandLine().IsViewMode(); // 2002/2/8 aroka ここに移動
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
			CAppNodeManager::getInstance()->GetNoNameNumber( pEditWnd->GetHwnd() );
			pEditWnd->UpdateCaption();
		}
	}

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

	if( hMutex ){
		::ReleaseMutex( hMutex );
		::CloseHandle( hMutex );
	}

	//プラグイン：EditorStartイベント実行
	CJackManager::getInstance()->InvokePlugins(PP_EDITOR_START, &pEditWnd->GetActiveView());

	// 2006.09.03 ryoji オープン後自動実行マクロを実行する
	if( !( bDebugMode || bGrepMode ) )
		pEditWnd->GetDocument()->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );

	// 起動時マクロオプション
	if (const auto macro = GetCCommandLine().GetMacro();
		pEditWnd->GetHwnd() && macro.has_value())
	{
		LPCWSTR pszMacro = macro.value();
		LPCWSTR pszMacroType = GetCCommandLine().GetMacroType().value_or(nullptr);
		CEditView& view = pEditWnd->GetActiveView();
		view.GetCommander().HandleCommand( F_EXECEXTMACRO, true, (LPARAM)pszMacro, (LPARAM)pszMacroType, 0, 0 );
	}

	//プラグイン：DocumentOpenイベント実行
	CJackManager::getInstance()->InvokePlugins( PP_DOCUMENT_OPEN, &pEditWnd->GetActiveView() );

	// 複数ファイル読み込み
	OpenFiles( pEditWnd->GetHwnd() );

	return pEditWnd->GetHwnd() ? true : false;
}

void CNormalProcess::InitProcess()
{
	// コントロールプロセスを起動する
	if (const auto profileName = GetCCommandLine().GetProfileOpt();
		!StartControlProcess(profileName))
	{
		throw process_init_failed( LS(STR_ERR_DLGNRMPROC2) ); // L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"
	}

	/* 共有メモリを初期化する */
	if (!InitShareData())
	{
		throw process_init_failed( LS(STR_ERR_DLGPROCESS1) ); // L"異なるバージョンのエディタを同時に起動することはできません。"
	}

// 微妙な仕様実装なので一旦コメントアウト
#if 0

	/* コマンドラインオプション */
	EditInfo fi;
	
	/* コマンドラインで受け取ったファイルが開かれている場合は */
	/* その編集ウィンドウをアクティブにする */
	GetCCommandLine().GetEditInfo(&fi); // 2002/2/8 aroka ここに移動

	if (HWND hwndOwner;
		fi.m_szPath[0] != L'\0' && GetCShareData().ActiveAlreadyOpenedWindow(fi.m_szPath, &hwndOwner, fi.m_nCharCode))
	{
		//	Oct. 27, 2000 genta
		//	MRUからカーソル位置を復元する操作はCEditDoc::FileLoadで
		//	行われるのでここでは必要なし．

		//	カーソル位置が引数に指定されていたら指定位置にジャンプ
		if (0 <= fi.m_ptCursor.y) {	//	行の指定があるか
			auto& pt = GetShareData().m_sWorkBuffer.m_LogicPoint;
			pt.y = fi.m_ptCursor.y;

			// 桁の指定が無い場合
			if (fi.m_ptCursor.x < 0) {
				SendMessageW(hwndOwner, MYWM_GETCARETPOS, 0, 0);
			} else {
				pt.x = fi.m_ptCursor.x;
			}
			SendMessageW(hwndOwner, MYWM_SETCARETPOS, 0, 0);
		}
		/* アクティブにする */
		ActivateFrameWindow(hwndOwner);

		// 複数ファイル読み込み
		OpenFiles(hwndOwner);

		return false;
	}
#endif

	//ドキュメントの作成
	m_pcEditDoc = std::make_unique<CEditDoc>();
	m_pcEditDoc->Create();

	m_pcLoadAgent = std::make_unique<CLoadAgent>();
	m_pcSaveAgent = std::make_unique<CSaveAgent>();
	m_pcVisualProgress = std::make_unique<CVisualProgress>();
	m_GrepAgent = std::make_unique<CGrepAgent>();	//GREPモード
	m_AppMode = std::make_unique<CAppMode>();	//編集モード
	m_pcMruListener = std::make_unique<CMruListener>();		//MRU管理

	m_MacroFactory = std::make_unique<CMacroFactory>();
	m_SMacroMgr = std::make_unique<CSMacroMgr>();	//マクロ管理

	// CEditAppを作成
	m_pcEditApp = std::make_unique<CEditApp>();

	SetMainWindow(std::make_unique<CEditWnd>());

	m_pcEditApp->m_pcEditWnd = GetEditWnd();
	m_pcEditApp->m_pcPropertyManager = GetEditWnd()->m_pcPropertyManager.get();

	m_pcEditDoc->m_cDocType.InitColorStrategyPool();

	// プラグイン読み込み
	GetPluginManager()->LoadAllPlugin();
}

bool CNormalProcess::InitShareData()
{
	const auto result = __super::InitShareData();
	if (result)
	{
		/* 言語を選択する */
		CSelectLang::ChangeLang(GetShareData().m_Common.m_sWindow.m_szLanguageDll);
	}
	return result;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief 複数ファイル読み込み

	@date 2015.03.14 novice 新規作成
*/
void CNormalProcess::OpenFiles( HWND hwnd )
{
	EditInfo fi;
	CCommandLine::getInstance()->GetEditInfo( &fi );
	bool bViewMode = CCommandLine::getInstance()->IsViewMode();

	int fileNum = CCommandLine::getInstance()->GetFileNum();
	if( fileNum > 0 ){
		int nDropFileNumMax = GetDllShareData().m_Common.m_sFile.m_nDropFileNumMax - 1;
		// ファイルドロップ数の上限に合わせる
		if( fileNum > nDropFileNumMax ){
			fileNum = nDropFileNumMax;
		}

		int i;
		for( i = 0; i < fileNum; i++ ){
			// ファイル名差し替え
			wcscpy( fi.m_szPath, CCommandLine::getInstance()->GetFileName(i) );
			bool ret = CControlTray::OpenNewEditor2( GetProcessInstance(), hwnd, &fi, bViewMode );
			if( ret == false ){
				break;
			}
		}

		// 用済みなので削除
		CCommandLine::getInstance()->ClearFile();
	}
}
