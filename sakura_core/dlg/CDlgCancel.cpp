/*!	@file
	@brief キャンセルボタンダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2008, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgCancel.h"

#include "apiwrap/StdApi.h"

CDlgCancel::CDlgCancel()
{
	m_bCANCEL = FALSE;	/* IDCANCELボタンが押された */
	m_bAutoCleanup = false;
}

/** 標準以外のメッセージを捕捉する
	@date 2008.05.28 ryoji 新規作成
*/
INT_PTR CDlgCancel::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_CLOSE:
		if( m_bAutoCleanup ){
			::DestroyWindow( GetHwnd() );
			return TRUE;
		}
		break;
	case WM_NCDESTROY:
		if( m_bAutoCleanup ){
			delete this;
			return TRUE;
		}
		break;
	}
	return result;
}

/** 自動破棄を遅延実行する
	@date 2008.05.28 ryoji 新規作成
*/
void CDlgCancel::DeleteAsync( void )
{
	m_bAutoCleanup = true;
	::PostMessageAny( GetHwnd(), WM_CLOSE, 0, 0 );
}

/* モーダルダイアログの表示 */
int CDlgCancel::DoModal( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCELボタンが押された */
	return (int)CDialog::DoModal( hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL );
}
/* モードレスダイアログの表示 */
HWND CDlgCancel::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCELボタンが押された */
	return CDialog::DoModeless( hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL, SW_SHOW );
}

bool CDlgCancel::OnInitDialog(HWND hwndDlg, HWND hWndFocus, LPARAM lParam)
{
	const auto hIcon = LoadIconW(nullptr, IDI_ASTERISK);
	SendMessageW(hwndDlg, WM_SETICON, ICON_SMALL, NULL);
	SendMessageW(hwndDlg, WM_SETICON, ICON_SMALL, LPARAM(hIcon));
	SendMessageW(hwndDlg, WM_SETICON, ICON_BIG, NULL);
	SendMessageW(hwndDlg, WM_SETICON, ICON_BIG, LPARAM(hIcon));

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog(hwndDlg, hWndFocus, lParam);
}

BOOL CDlgCancel::OnBnClicked( int wID )
{
	switch( wID ){
	case IDCANCEL:
		m_bCANCEL = TRUE;	/* IDCANCELボタンが押された */
//		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID CDlgCancel::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end
