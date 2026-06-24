/*!	@file
	@brief バージョン情報ダイアログ

	@author Norio Nakatani
	@date 1998/05/22 作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDLGABOUT_7F887984_7DEB_42C7_AB87_7CE7D9801700_H_
#define SAKURA_CDLGABOUT_7F887984_7DEB_42C7_AB87_7CE7D9801700_H_
#pragma once

#include "dlg/CDialog.h"
#include "window/CWnd.h"

/*!
	@brief About Box管理
	
	DispatchEventを独自に定義することで，CDialogでサポートされていない
	メッセージを捕捉する．
*/

class CUrlWnd final : public CCustomizedWnd
{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;

	using Base = CCustomizedWnd;
	using Me = CDialog;

public:
	CUrlWnd() = default;

	BOOL SetSubclassWindow( HWND hWnd );

	HFONT	GetFont() const noexcept { return m_hFont; }

	LRESULT	DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void	OnSetText(HWND hWnd, _In_z_ LPCWSTR pchText) const;

	FontHolder	m_hFont = nullptr;
	BOOL m_bHilighted = FALSE;
};

class CDlgAbout final : public CDialog
{
public:
	int DoModal(HINSTANCE hInstance, HWND hwndParent);	/* モーダルダイアログの表示 */
	//	Nov. 7, 2000 genta	標準以外のメッセージを捕捉する
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;
protected:
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnBnClicked(int wID) override;
	BOOL OnStnClicked(int wID) override;
	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add
private:
	CUrlWnd m_UrlUrWnd;
	CUrlWnd m_UrlGitWnd;
	CUrlWnd m_UrlBuildLinkWnd;
	CUrlWnd m_UrlGitHubCommitWnd;
	CUrlWnd m_UrlGitHubPRWnd;
	CUrlWnd m_UrlOrgWnd;
};
#endif /* SAKURA_CDLGABOUT_7F887984_7DEB_42C7_AB87_7CE7D9801700_H_ */
