/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
 */
#include "StdAfx.h"
#include "dlg/CDlgInput1.h"

#include "CEditApp.h"
#include "Funccode_enum.h"	// EFunctionCode
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "util/window.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"

// 入力 CDlgInput1.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13000
	IDOK,					HIDOK_DLG1,
	IDCANCEL,				HIDCANCEL_DLG1,
	IDC_EDIT_INPUT1,		HIDC_DLG1_EDIT1,	//入力フィールド	IDC_EDIT1->IDC_EDIT_INPUT1	2008/7/3 Uchi
	IDC_STATIC_MSG,			HIDC_DLG1_EDIT1,	//メッセージ
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgInput1::CDlgInput1()
	: CDialog(false, false)
{
}

/* モーダルダイアログの表示 */
bool CDlgInput1::DoModal(
	HWND				hwndParent,
	std::wstring_view	title,
	std::wstring_view	message,
	std::span<WCHAR>	buffer
)
{
	m_Title = title;
	m_Message = message;
	m_Text = buffer;

	const auto unusedArg1 = nullptr;
	return (BOOL) CDialog::DoModal(unusedArg1, hwndParent, IDD_INPUT1, 0L);
}

/* モーダルダイアログの表示 */
BOOL CDlgInput1::DoModal(
	HINSTANCE		hInstApp,
	HWND			hwndParent,
	const WCHAR*	pszTitle,
	const WCHAR*	pszMessage,
	size_t			bufferSize,
	WCHAR*			pszText
)
{
	return DoModal(hwndParent, pszTitle, pszMessage, std::span<WCHAR>(pszText, bufferSize + 1));
}

int CDlgInput1::GetData()
{
	const auto hWndDlg = GetHwnd();

	auto result = ApiWrap::GetDlgItemTextW(hWndDlg, IDC_EDIT_INPUT1);
	if (!result) return -1;

	if (std::size(m_Text) <= std::size(result.text)) return -1;

	::wcscpy_s(std::data(m_Text), std::size(m_Text), std::data(result.text));

	return 1;
}

/* ダイアログのメッセージ処理 */
INT_PTR	CDlgInput1::DispatchDlgEvent(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const auto hwndDlg = hWndDlg;
	const auto uMsg = uMsg;
	switch (uMsg) {
	//@@@ 2002.01.07 add start
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	//@@@ 2002.01.07 add end

	default:
		break;
	}

	return CDialog::DispatchEvent(hwndDlg, uMsg, wParam, lParam);
}

bool CDlgInput1::OnInitDialog(HWND hWndDlg, HWND hWndFocus, LPARAM lParam)
{
	const auto bRet = CDialog::OnInitDialog(hWndDlg, hWndFocus, lParam);

	ApiWrap::SetWindowTextW(hWndDlg, m_Title);
	ApiWrap::SetDlgItemTextW(hWndDlg, IDC_STATIC_MSG, m_Message);

	ApiWrap::LimitEditText(::GetDlgItem(hWndDlg, IDC_EDIT_INPUT1), m_Text);
	ApiWrap::SetDlgItemTextW(hWndDlg, IDC_EDIT_INPUT1, std::data(m_Text));

	return bRet;
}

BOOL CDlgInput1::OnBnClicked(int wID)
{
	const auto hWndDlg = GetHwnd();

	if (IDCANCEL == wID) {
		::EndDialog(hWndDlg, FALSE);
		return TRUE;
	}

	if (GetData() < 0) {
		return TRUE;
	}

	if (IDOK != wID) {
		return FALSE;
	}

	::EndDialog(hWndDlg, TRUE);
	return TRUE;
}