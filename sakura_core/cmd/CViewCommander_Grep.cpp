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

#include "CEditApp.h"
#include "CGrepAgent.h"
#include "plugin/CPlugin.h"
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
	const auto hWnd = m_pCommanderView->GetHwnd();
	const auto& cDocFile = GetDocument()->m_cDocFile;
	if (const auto nRet = cDlgGrep.DoModal(G_AppInstance(), hWnd, cDocFile.GetFilePathClass().IsValidPath() ? cDocFile.GetFilePath() : nullptr); !nRet) {
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

	CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
		m_pCommanderView,
		cDlgGrep
	);
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

	const auto hWnd = m_pCommanderView->GetHwnd();
	const auto& cDocFile = GetDocument()->m_cDocFile;
	if (const auto nRet = cDlgGrep.DoModal(G_AppInstance(), hWnd, cDocFile.GetFilePathClass().IsValidPath() ? cDocFile.GetFilePath() : nullptr); !nRet) {
		return;
	}

	HandleCommand(F_GREP_REPLACE, TRUE, 0, 0, 0, 0);	//	GREPコマンドの発行
}

/*! GREP置換実行
*/
void CViewCommander::Command_GREP_REPLACE( void )
{
	auto& cDlgGrep = GetEditWindow()->m_cDlgGrepReplace;

	CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
		m_pCommanderView,
		cDlgGrep
	);
}
