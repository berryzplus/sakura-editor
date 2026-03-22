/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_
#define SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_
#pragma once

#include "dlg/CDialog.h"
#include "mem/CNativeW.h"

class CDlgInput1;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief １行入力ダイアログボックス
*/
class CDlgInput1 final : public CDialog
{
private:
	using Me = CDlgInput1;

	using CDialog::DoModal;

public:
	CDlgInput1();

	bool	DoModal(HWND hWndParent, std::wstring_view title, std::wstring_view message, std::span<WCHAR> buffer);

	BOOL DoModal( HINSTANCE hInstApp, HWND hwndParent, const WCHAR* pszTitle,
				  const WCHAR* pszMessage, size_t bufferSize, WCHAR* pszText );

	int		GetData() override;

	/*
	||  Attributes & Operations
	*/
	INT_PTR	DispatchEvent(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	bool	OnInitDialog(HWND hWndDlg, HWND hWndFocus, LPARAM lParam) override;
	BOOL	OnBnClicked(int wID) override;

private:
	std::wstring_view	m_Title;			//!< ダイアログタイトル
	std::wstring_view	m_Message;			//!< メッセージ
	std::span<WCHAR>	m_Text;				//!< テキスト
};

#endif /* SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_ */
