/*!	@file
	@brief 分割ボックスウィンドウクラス

	@author Norio Nakatani

	@date 2002/2/3 aroka 未使用コード除去
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CSplitBoxWnd.h"
#include "uiparts/CGraphics.h"
#include "apiwrap/StdApi.h"
#include "apiwrap/DarkMode.h"
#include "config/system_constants.h"

namespace window {

std::wstring SplitBoxClassName(bool bVertical)
{
	if (bVertical) {
		return L"VSplitBoxWnd";
	} else {
		return L"HSplitBoxWnd";
	}
}

LPCWSTR SplitBoxCursorName(bool bVertical){
	if (bVertical) {
		return IDC_SIZENS;
	} else {
		return IDC_SIZEWE;
	}
}

} // namespace window

CSplitBoxWnd::CSplitBoxWnd(bool bVertical)
	: COriginalWnd(window::SplitBoxClassName(bVertical))
	, m_bVertical(bVertical)
	, m_CursorName(window::SplitBoxCursorName(bVertical))
{
	return;
}

CSplitBoxWnd::~CSplitBoxWnd()
{
	return;
}

HWND CSplitBoxWnd::Create( HINSTANCE hInstance, HWND hwndParent, int bVertical )
{
	UNREFERENCED_PARAMETER(hInstance);

	/* ウィンドウクラス作成 */
	const auto atom = RegisterClassW(HBRUSH(COLOR_3DFACE + 1), ::LoadCursorW(nullptr, m_CursorName));

	m_bVertical = bVertical;

	/* 親ウィンドウのクライアント領域のサイズを取得 */
	CMyRect rc;
	::GetClientRect( GetParentHwnd(), &rc );

	/* システムマトリックスの取得 */
	const auto cyHScroll = GetSystemMetrics(SM_CYHSCROLL);	/* 水平スクロールバーの高さ */
	const auto cxVScroll = GetSystemMetrics(SM_CXVSCROLL);	/* 垂直スクロールバーの幅 */

	if (bVertical) {
		rc.SetPos(rc.Width() - cxVScroll, 0);
		rc.SetSize(cxVScroll, 7);
	} else {
		rc.SetPos(0, rc.Height() - cyHScroll);
		rc.SetSize(7, cyHScroll);
	}

	return Base::CreateWnd(atom, WS_CHILD | WS_VISIBLE, hwndParent, 0, rc, m_ClassName);
}

/* 描画処理 */
void CSplitBoxWnd::Draw3dRect( HDC hdc, int x, int y, int cx, int cy,
	COLORREF clrTopLeft, COLORREF clrBottomRight )
{
	RECT	rc;
	::SetRect( &rc, x, y, x + cx - 1, y + 1 );
	::MyFillRect( hdc, rc, clrTopLeft );
	::SetRect( &rc, x, y, x + 1, y + cy - 1 );
	::MyFillRect( hdc, rc, clrTopLeft );

	::SetRect( &rc, x + cx - 1, y, x + cx, y + cy );
	::MyFillRect( hdc, rc, clrBottomRight );
	::SetRect( &rc, x, y + cy - 1, x + cx, y + cy );
	::MyFillRect( hdc, rc, clrBottomRight );
	return;
}

void CSplitBoxWnd::FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr )
{
	RECT	rc;
	::SetBkColor( hdc, clr );
	::SetRect( &rc, x, y, x + cx, y + cy );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, nullptr, 0, nullptr );
	return;
}

/*!
 * @brief WM_PAINTハンドラ
 *
 * WM_PAINTはウィンドウの描画中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 * @note windowsx.h の定義が微妙なので独自に定義
 */
void CSplitBoxWnd::OnPaint(HWND hWnd, PAINTSTRUCT& ps)
{
	UNREFERENCED_PARAMETER(hWnd);

	const auto hdc = ps.hdc;

	const auto cxBorder  = ::GetSystemMetrics(SM_CXBORDER);
	const auto cyBorder  = ::GetSystemMetrics(SM_CYBORDER);
	const auto cxEdge    = ::GetSystemMetrics(SM_CXEDGE);
	const auto cyEdge    = ::GetSystemMetrics(SM_CYEDGE);
	const auto cyHScroll = ::GetSystemMetrics(SM_CYHSCROLL);
	const auto cxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
	const auto cxHSplit  = cxEdge * 2 + cxBorder * 3;	/* 水平分割ボックスの幅 */
	const auto cyVSplit  = cyEdge * 2 + cyBorder * 3;	/* 垂直分割ボックスの高さ */

	COLORREF cBTN = ::GetSysColor(COLOR_BTNFACE);
	COLORREF cBR0 = ::GetSysColor(COLOR_3DSHADOW);
	COLORREF cBR1 = ::GetSysColor(COLOR_BTNSHADOW);

	if (IsDarkModeActive()) {
		cBTN = DarkMode::getCtrlBackgroundColor();
		cBR0 = DarkMode::getTextColor();
		cBR1 = DarkMode::getHotEdgeColor();
	}

	RECT rc{};

	if (m_bVertical) {
		/* 垂直分割ボックスの描画 */
		::SetRect(&rc, cxEdge, cyEdge, cxVScroll - cxEdge, cyVSplit - cyEdge);
		::MyFillRect(hdc, rc, cBTN);

		::SetRect(&rc, cxEdge, cyVSplit - cyEdge, cxVScroll - cxEdge, cyVSplit);
		::MyFillRect(hdc, rc, cBR0);
		::SetRect(&rc, cxVScroll - cxEdge, cyEdge, cxVScroll, cyVSplit);
		::MyFillRect(hdc, rc, cBR1);

		::SetRect(&rc, cxEdge, 0, cxVScroll - cxEdge, cyEdge);
		::MyFillRect(hdc, rc, cBR0);
		::SetRect(&rc, 0, 0, cxEdge, cyVSplit - cyEdge);
		::MyFillRect(hdc, rc, cBR1);

	}else{
		/* 水平分割ボックスの描画 */
		::SetRect(&rc, cxEdge, cyEdge, cxHSplit - cxEdge, cyHScroll - cyEdge);
		::MyFillRect(hdc, rc, cBTN);

		::SetRect(&rc, cxHSplit - cxEdge, cyEdge, cxHSplit, cyHScroll);
		::MyFillRect(hdc, rc, cBR0);
		::SetRect(&rc, 0, cyHScroll - cyEdge, cxHSplit - cxEdge, cyHScroll);
		::MyFillRect(hdc, rc, cBR1);

		::SetRect(&rc, 0, 0, cxEdge, cyHScroll - cyEdge);
		::MyFillRect(hdc, rc, cBR0);
		::SetRect(&rc, cxEdge, 0, cxHSplit, cyEdge);
		::MyFillRect(hdc, rc, cBR1);
	}
}

//WM_LBUTTONDOWN
void CSplitBoxWnd::OnLButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	if (fDoubleClick) {
		OnLButtonDblClk(hWnd, x, y, keyFlags);
		return;
	}

	const auto hwnd = hWnd;

	HDC			hdc;
	RECT		rc;
	RECT		rc2;
	int			nCyHScroll;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;

	if (!hWnd) {
		return;
	}

	::SetCapture( hwnd );
	if( m_bVertical ){
		m_nDragPosY = 1;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* 水平スクロールバーの高さ */
		rc.bottom -= nCyHScroll;

		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + 6;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );
	}else{
		m_nDragPosX = 1;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );

		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + 6;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );
	}
}

//WM_MOUSEMOVE
void CSplitBoxWnd::OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);

	HDC			hdc;
	int			xPos;
	int			yPos;
	RECT		rc;
	RECT		rc2;
	int			nCyHScroll;
	int			nCxVScroll;
	POINT		po;
	POINT		po_top;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;

	if (::GetCapture() == hWnd) {
		return;
	}

	if( m_bVertical ){
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* 水平スクロールバーの高さ */
		rc.bottom -= nCyHScroll;

		::GetCursorPos( &po );

		po_top.x = 0;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.y < po_top.y ){
			po.y = po_top.y;
		}

		po_top.x = 0;
		po_top.y = rc.bottom;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.y > po_top.y - 6 ){
			po.y = po_top.y - 6;
		}

		::ScreenToClient( ::GetParent( GetParentHwnd() ), &po );
		xPos = po.x;
		yPos = po.y;

		if( yPos != m_nDragPosY ){
//			MYTRACE( L"xPos=%d yPos=%d\n", xPos, yPos );

			hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
			::SetBkColor( hdc, RGB(0, 0, 0) );
			hBrush = ::CreateSolidBrush( RGB(255,255,255) );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

			::SetROP2( hdc, R2_XORPEN );
			::SetBkMode( hdc, TRANSPARENT );
			rc2.left = -1;
			rc2.top = m_nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + 6;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			m_nDragPosY =  po.y;

			rc2.left = -1;
			rc2.top = m_nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + 6;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			::SelectObject( hdc, hBrushOld );
			::DeleteObject( hBrush );
			::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );
		}
	}else{
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* 垂直スクロールバーの幅 */
		rc.right -= nCxVScroll;

		::GetCursorPos( &po );

		po_top.x = 0;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.x < po_top.x ){
			po.x = po_top.x;
		}

		po_top.x = rc.right;
		po_top.y = 0;
		::ClientToScreen( ::GetParent( GetParentHwnd() ), &po_top );
		if( po.x > po_top.x - 6 ){
			po.x = po_top.x - 6;
		}

		::ScreenToClient( ::GetParent( GetParentHwnd() ), &po );
		xPos = po.x;
		yPos = po.y;

		if( xPos != m_nDragPosX ){
//			MYTRACE( L"xPos=%d yPos=%d\n", xPos, yPos );

			hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
			::SetBkColor( hdc, RGB(0, 0, 0) );
			hBrush = ::CreateSolidBrush( RGB(255,255,255) );
			hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

			::SetROP2( hdc, R2_XORPEN );
			::SetBkMode( hdc, TRANSPARENT );

			rc2.left = m_nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + 6;
			rc2.bottom = rc.bottom;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			m_nDragPosX =  po.x;

			rc2.left = m_nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + 6;
			rc2.bottom = rc.bottom;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

			::SelectObject( hdc, hBrushOld );
			::DeleteObject( hBrush );
			::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );
		}
	}
}

//WM_LBUTTONUP
void CSplitBoxWnd::OnLButtonUp( HWND hWnd, int x, int y, UINT keyFlags )
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(keyFlags);

	HDC			hdc;
	RECT		rc;
	RECT		rc2;
	int			nCyHScroll;
	int			nCxVScroll;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;

	if (::GetCapture() == hWnd) {
		return;
	}

	if( m_bVertical ){
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* 水平スクロールバーの高さ */
		rc.bottom -= nCyHScroll;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );
		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + 6;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( ::GetParent( GetParentHwnd() ), hdc );

		/* 親ウィンドウに、メッセージをポストする */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)0, (LPARAM)m_nDragPosY );

	}else{
		::GetClientRect( ::GetParent( GetParentHwnd() ), &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );	/* 垂直スクロールバーの幅 */
		rc.right -= nCxVScroll;

		hdc = ::GetDC( ::GetParent( GetParentHwnd() ) );
		::SetBkColor( hdc, RGB(0, 0, 0) );
		hBrush = ::CreateSolidBrush( RGB(255,255,255) );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );

		::SetROP2( hdc, R2_XORPEN );
		::SetBkMode( hdc, TRANSPARENT );

		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + 6;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );

		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( GetParentHwnd(), hdc );

		/* 親ウィンドウに、メッセージをポストする */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)m_nDragPosX, (LPARAM)0 );
	}
	::ReleaseCapture();
}

//WM_LBUTTONDBLCLK
void CSplitBoxWnd::OnLButtonDblClk(HWND hWnd, int x, int y, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(keyFlags);

	RECT		rc;
	int			nCyHScroll;

	if (!hWnd) {
		return;
	}

	if( m_bVertical ){
		::GetClientRect( GetParentHwnd(), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* 水平スクロールバーの高さ */
		rc.bottom -= nCyHScroll;

		/* 親ウィンドウに、メッセージをポストする */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)0, (LPARAM)(rc.bottom / 2) );
	}
	else{
		::GetClientRect( GetParentHwnd(), &rc );
		nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );	/* 水平スクロールバーの高さ */
		rc.bottom -= nCyHScroll;

		/* 親ウィンドウに、メッセージをポストする */
		::PostMessageAny( GetParentHwnd(), MYWM_DOSPLIT, (WPARAM)(rc.right / 2), (LPARAM)0 );
	}
}
