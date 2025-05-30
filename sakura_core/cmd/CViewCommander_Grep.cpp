/*!	@file
@brief CViewCommanderクラスのコマンド(Grep)関数群

*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "plugin/CPlugin.h"
#include "plugin/CJackManager.h"
#include "CSelectLang.h"
#include "String_define.h"

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
 */
void CViewCommander::Command_GREP_DIALOG( void )
{
	auto& cDlgGrep = GetEditWindow()->m_cDlgGrep;

	// 2014.07.01 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = !cDlgGrep.m_bSetText;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	if (CNativeW cmemCurText; m_pCommanderView->GetCurrentTextForSearchDlg(cmemCurText, bGetHistory)) {
		cDlgGrep.m_strText = cmemCurText.GetStringPtr();
		cDlgGrep.m_bSetText = true;
	}

	/* Grepダイアログの表示 */
	if (const auto nRet = cDlgGrep.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath()); !nRet) {
		return;
	}

	HandleCommand(F_GREP, true, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP実行

	@date 2005.01.10 genta CEditView_Commandより移動
*/
void CViewCommander::Command_GREP( void )
{
	auto& cDlgGrep = GetEditWindow()->m_cDlgGrep;

	CNativeW		cmWork1;
	CNativeW		cmWork2;
	CNativeW		cmWork3;
	CNativeW		cmWork4;
	cmWork1.SetString( cDlgGrep.m_strText.c_str() );
	cmWork2 = cDlgGrep.GetPackedGFileString();
	cmWork3.SetString( cDlgGrep.m_szFolder );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if( (  CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* 現在編集中のファイルのパス */
		  !CAppMode::getInstance()->IsDebugMode()
		)
	){
		// 2011.01.23 Grepタイプ別適用
		if( !GetDocument()->m_cDocEditor.IsModified() && GetDocument()->m_cDocLineMgr.GetLineCount() == 0 ){
			CTypeConfig cTypeGrep = CDocTypeManager().GetDocumentTypeOfExt( L"grepout" );
			const STypeConfigMini* pConfig = NULL;
			if( !CDocTypeManager().GetTypeConfigMini( cTypeGrep, &pConfig ) ){
				return;
			}
			GetDocument()->m_cDocType.SetDocumentTypeIdx( pConfig->m_id );
			GetDocument()->m_cDocType.LockDocumentType();
			GetDocument()->OnChangeType();
		}
		
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			false,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			cDlgGrep.m_bSubFolder,
			false,
			true, // Header
			cDlgGrep.m_sSearchOption,
			cDlgGrep.m_nGrepCharSet,
			cDlgGrep.m_nGrepOutputLineType,
			cDlgGrep.m_nGrepOutputStyle,
			cDlgGrep.m_bGrepOutputFileOnly,
			cDlgGrep.m_bGrepOutputBaseFolder,
			cDlgGrep.m_bGrepSeparateFolder,
			false,
			false
		);

		//プラグイン：DocumentOpenイベント実行
		CJackManager::getInstance()->InvokePlugins( PP_DOCUMENT_OPEN, &GetEditWindow()->GetActiveView() );
	}
	else{
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		CControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), cDlgGrep);
	}
	return;
}

/*! GREP置換ダイアログの表示
 */
void CViewCommander::Command_GREP_REPLACE_DLG( void )
{
	auto& cDlgGrep = GetEditWindow()->m_cDlgGrepReplace;

	// 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = !cDlgGrep.m_bSetText;

	if (CNativeW cmemCurText; m_pCommanderView->GetCurrentTextForSearchDlg(cmemCurText, bGetHistory)) {
		cDlgGrep.m_strText = cmemCurText.GetStringPtr();
		cDlgGrep.m_bSetText = true;
	}

	if( 0 < GetDllShareData().m_sSearchKeywords.m_aReplaceKeys.size() ){
		if( cDlgGrep.m_nReplaceKeySequence < GetDllShareData().m_Common.m_sSearch.m_nReplaceKeySequence ){
			cDlgGrep.m_strText2 = GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0];
		}
	}

	if (const auto nRet = cDlgGrep.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath(), (LPARAM)m_pCommanderView); !nRet) {
		return;
	}

	HandleCommand(F_GREP_REPLACE, TRUE, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP置換実行
*/
void CViewCommander::Command_GREP_REPLACE( void )
{
	auto& cDlgGrep = GetEditWindow()->m_cDlgGrepReplace;

	CNativeW		cmWork1;
	CNativeW		cmWork2;
	CNativeW		cmWork3;
	CNativeW		cmWork4;

	cmWork1.SetString( cDlgGrep.m_strText.c_str() );
	cmWork2 = cDlgGrep.GetPackedGFileString();
	cmWork3.SetString( cDlgGrep.m_szFolder );
	cmWork4.SetString( cDlgGrep.m_strText2.c_str() );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if( (  CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* 現在編集中のファイルのパス */
		  !CAppMode::getInstance()->IsDebugMode()
		)
	){
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			cDlgGrep.m_bSubFolder,
			false, // Stdout
			true, // Header
			cDlgGrep.m_sSearchOption,
			cDlgGrep.m_nGrepCharSet,
			cDlgGrep.m_nGrepOutputLineType,
			cDlgGrep.m_nGrepOutputStyle,
			cDlgGrep.m_bGrepOutputFileOnly,
			cDlgGrep.m_bGrepOutputBaseFolder,
			cDlgGrep.m_bGrepSeparateFolder,
			cDlgGrep.m_bPaste,
			cDlgGrep.m_bBackup
		);
	}
	else{
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		CControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), cDlgGrep);
	}
	return;
}
