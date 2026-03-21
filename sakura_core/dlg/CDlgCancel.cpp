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
INT_PTR	CDlgCancel::DispatchDlgEvent(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const auto hWnd = hWndDlg;
	const auto wMsg = uMsg;
	INT_PTR result;
	result = CDialog::DispatchDlgEvent(hWndDlg, uMsg, wParam, lParam);
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
	default:
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
	return (int)CDialog::DoModal( hInstance, hwndParent, nDlgTemplete, (LPARAM)nullptr );
}
/* モードレスダイアログの表示 */
HWND CDlgCancel::DoModeless( HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete )
{
	m_bCANCEL = FALSE;	/* IDCANCELボタンが押された */
	return CDialog::DoModeless( hInstance, hwndParent, nDlgTemplete, (LPARAM)nullptr, SW_SHOW );
}

bool CDlgCancel::OnInitDialog(HWND hWndDlg, HWND hWndFocus, LPARAM lParam)
{
	const auto bRet = CDialog::OnInitDialog(hWndDlg, hWndFocus, lParam);

	HICON	hIcon;
	hIcon = ::LoadIcon( nullptr, IDI_ASTERISK );
//	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_GREP ) );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)nullptr );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)nullptr );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIcon );

	return bRet;
}

BOOL CDlgCancel::OnBnClicked( int wID )
{
	switch( wID ){
	case IDCANCEL:
		m_bCANCEL = TRUE;	/* IDCANCELボタンが押された */
//		CloseDialog( 0 );
		return TRUE;
	default:
		break;
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
