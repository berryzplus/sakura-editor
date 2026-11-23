/*!	@file
@brief CViewCommanderクラスのコマンド(挿入系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "dlg/CDlgCtrlCode.h"	//コントロールコードの入力(ダイアログ)
#include "env/CFormatManager.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"

//日付挿入
void CViewCommander::Command_INS_DATE( void )
{
	// 日付をフォーマット
	StaticString<1024> szText{};
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetDateFormat( systime, szText, int(std::size(szText)) - 1 );

	// テキストを挿入する
	_ReplaceSel(szText.str());
}

//時刻挿入
void CViewCommander::Command_INS_TIME( void )
{
	// 時刻をフォーマット
	StaticString<1024> szText{};
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetTimeFormat( systime, szText, int(std::size(szText)) - 1 );

	// テキストを挿入する
	_ReplaceSel(szText.str());
}

//	from CViewCommander_New.cpp
/*!	コントロールコードの入力(ダイアログ)
	@author	MIK
	@date	2002/06/02
*/
void CViewCommander::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//コントロールコード入力ダイアログを表示する
	if( cDlgCtrlCode.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		//コントロールコードを入力する
		// 2013.06.11 Command_WCHAR -> HandleCommand マクロ記録対応
		// 2013.12.12 F_WCHAR -> F_CTRL_CODE
		HandleCommand( F_CTRL_CODE, true, cDlgCtrlCode.GetCharCode(), 0, 0, 0 );
	}
}

// 最近使ったファイル挿入
void CViewCommander::Command_INS_FILE_USED_RECENTLY( void )
{
	std::wstring eol = GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2();
	std::wstring s;
	CMRUFile cMRUFile;
	auto list = cMRUFile.GetPathList();
	for (auto item : list) {
		s += item;
		s += eol;
	}

	// テキストを挿入する
	_ReplaceSel(s);
}

// 最近使ったフォルダー挿入
void CViewCommander::Command_INS_FOLDER_USED_RECENTLY( void )
{
	std::wstring eol = GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2();
	std::wstring s;
	CMRUFolder cMRUFolder;
	auto list = cMRUFolder.GetPathList();
	for (auto item : list) {
		s += item;
		s += eol;
	}

	// テキストを挿入する
	_ReplaceSel(s);
}

/*!
 * @brief テキストを挿入する
 *
 * 現在のキャレット位置に指定したテキストを挿入します。
 * Windows標準 EM_REPLACESEL に相当します。
 *
 * @param[in] text 挿入するテキスト
 * @param[in, opt] bSelectInsertedText 挿入したテキストを選択するかどうか。省略時は選択しない。
 */
void CViewCommander::_ReplaceSel(
	std::wstring_view text,		//!< [in] 挿入するテキスト
	bool bSelectInsertedText	//!< [in, opt] 挿入したテキストを選択するかどうか。省略時は選択しない。
)
{
	// 選択範囲の選択解除
	if (bSelectInsertedText &&
		GetSelectionInfo().IsTextSelected())
	{
		GetSelectionInfo().DisableSelectArea(true);
	}

	// 挿入前のキャレット位置を記憶する
	// ドラッグ＆ドロップではキャレット位置が行終端より右側になることはない。
	const auto logicPos = GetCaret().GetCaretLogicPos();

	// テキストを貼り付け
	Command_INSTEXT(
		true,
		text.data(),
		ptrdiff_t(text.length()),
		TRUE,
		false,		//!< [in, opt] bLinePaste ラインモード貼り付け。🌟そのうち削除します。
		false,		//!< [in, opt] bFastMode 高速モード🌟そのうち削除します。
		nullptr		//!< [out, opt] psDelRangeLogicFast 高速モードのときの削除範囲ロジック単位。🌟そのうち削除します。
	);

	// 挿入したテキストを選択する
	if (bSelectInsertedText) {
		CLayoutPoint ptSelectFrom;
		GetLayoutMgr().LogicToLayout(
			logicPos,
			&ptSelectFrom
		);

		GetSelectionInfo().SetSelectArea( CLayoutRange(ptSelectFrom, GetCaret().GetCaretLayoutPos()) );
		GetSelectionInfo().DrawSelectArea();
	}
}
