/*!	@file
	@brief ウィンドウの基本クラス

	@author Norio Nakatani
	@date 2000/01/11 新規作成
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CWnd.h"

#include "_main/global.h"

CWnd::CWnd(std::wstring_view className, bool isSakuraStyle) noexcept
	: COriginalWnd(isSakuraStyle ? fmt::format(L"CWnd{}", className) : className, G_AppInstance())
{
}

/* ウィンドウクラス作成 */
ATOM CWnd::RegisterWC(
	HINSTANCE	hInstance,
	HICON		hIcon,			// Handle to the class icon.
	HICON		hIconSm,		// Handle to a small icon
	HCURSOR		hCursor,		// Handle to the class cursor.
	HBRUSH		hbrBackground,	// Handle to the class background brush.
	LPCWSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
	LPCWSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(lpszClassName);

	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	return RegisterWnd(
		hCursor,
		hbrBackground,
		CS_DBLCLKS,
		hIcon,
		hIconSm,
		lpszMenuName
	);
}

/* 作成 */
HWND CWnd::Create(
	HWND		hwndParent,
	DWORD		dwExStyle,		// extended window style
	LPCWSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
	LPCWSTR		lpWindowName,	// pointer to window name
	DWORD		dwStyle,		// window style
	int			x,				// horizontal position of window
	int			y,				// vertical position of window
	int			nWidth,			// window width
	int			nHeight,		// window height
	HMENU		hMenu			// handle to menu, or child-window identifier
)
{
	UNREFERENCED_PARAMETER(lpszClassName);

	const RECT rcDesired = { x, y, x + nWidth, y + nHeight };

	m_hWnd = CreateWnd(
		hwndParent,
		UINT(size_t(hMenu)),
		dwStyle,
		dwExStyle,
		lpWindowName,
		rcDesired
	);

	if( NULL == m_hWnd ){
		::MessageBox( m_hwndParent, L"CWnd::Create()\n\n::CreateWindowEx failed.", L"error", MB_OK );
		return NULL;
	}

	return m_hWnd;
}

/* メッセージ配送 */
LRESULT CWnd::DispatchEvent( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	const auto hWnd = hwnd;
	const auto uMsg = msg;
	const auto wParam = wp;
	const auto lParam = lp;

	#define CALLH(message, method) case message: return method( hwnd, msg, wp, lp )
	switch( msg ){
	CALLH( WM_DESTROY			, OnDestroy			);
	CALLH( WM_SIZE				, OnSize			);
	CALLH( WM_COMMAND			, OnCommand			);
	CALLH( WM_LBUTTONDOWN		, OnLButtonDown		);
	CALLH( WM_LBUTTONUP			, OnLButtonUp		);
	CALLH( WM_LBUTTONDBLCLK		, OnLButtonDblClk	);
	CALLH( WM_RBUTTONDOWN		, OnRButtonDown		);
	CALLH( WM_MBUTTONDOWN		, OnMButtonDown		);
	CALLH( WM_MOUSEMOVE			, OnMouseMove		);
	CALLH( WM_PAINT				, OnPaint			);
	CALLH( WM_TIMER				, OnTimer			);

	CALLH( WM_MEASUREITEM		, OnMeasureItem		);
	CALLH( WM_NOTIFY			, OnNotify			);	//@@@ 2003.05.31 MIK
	CALLH( WM_DRAWITEM			, OnDrawItem		);	// 2006.02.01 ryoji
	CALLH( WM_CAPTURECHANGED	, OnCaptureChanged	);	// 2006.11.30 ryoji

	default:
		if( WM_APP <= msg && msg <= 0xBFFF ){
			/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
			return DispatchEvent_WM_APP( hwnd, msg, wp, lp );
		}
		break;	/* default */
	}

	return __super::DispatchEvent(hWnd, uMsg, wParam, lParam);
}

/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
LRESULT CWnd::DispatchEvent_WM_APP( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return CallDefWndProc( hwnd, msg, wp, lp );
}

/* デフォルトメッセージ処理 */
LRESULT CWnd::CallDefWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	return ::DefWindowProc( hwnd, msg, wp, lp );
}

/* ウィンドウを破棄 */
void CWnd::DestroyWindow()
{
	if(m_hWnd){
		::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	}
}
