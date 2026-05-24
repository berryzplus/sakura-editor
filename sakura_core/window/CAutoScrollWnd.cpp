/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "window/CAutoScrollWnd.h"

#include "view/CEditView.h"

#include "sakura_rc.h"

namespace window {

std::wstring AutoScrollClassName(bool bVertical, bool bHorizontal)
{
	if (bVertical && bHorizontal) {
		return L"SakuraAutoScrollCWnd";
	} else if (bVertical) {
		return L"SakuraAutoScrollVWnd";
	} else {
		return L"SakuraAutoScrollHWnd";
	}
}

int AutoScrollBitmapId(bool bVertical, bool bHorizontal)
{
	if (bVertical && bHorizontal) {
		return IDB_SCROLL_CENTER;
	} else if (bVertical) {
		return IDB_SCROLL_VERTICAL;
	} else {
		return IDB_SCROLL_HORIZONTAL;
	}
}

int AutoScrollCursorId(bool bVertical, bool bHorizontal)
{
	if (bVertical && bHorizontal) {
		return IDC_CURSOR_AUTOSCROLL_CENTER;
	} else if (bVertical) {
		return IDC_CURSOR_AUTOSCROLL_VERTICAL;
	} else {
		return IDC_CURSOR_AUTOSCROLL_HORIZONTAL;
	}
}

} // namespace window

/* static */ std::unique_ptr<CAutoScrollWnd> CAutoScrollWnd::CreateInstance(bool bVertical, bool bHorizontal)
{
	if (bVertical && bHorizontal) {
		return std::make_unique<CAutoScrollCWnd>();
	} else if (bVertical) {
		return std::make_unique<CAutoScrollVWnd>();
	} else {
		return std::make_unique<CAutoScrollHWnd>();
	}
}

CAutoScrollWnd::CAutoScrollWnd(bool bVertical, bool bHorizontal)
	: COriginalWnd(window::AutoScrollClassName(bVertical, bHorizontal))
	, m_BitMapId(window::AutoScrollBitmapId(bVertical, bHorizontal))
	, m_CursorId(window::AutoScrollCursorId(bVertical, bHorizontal))
{
	return;
}

CAutoScrollWnd::~CAutoScrollWnd()
{
	return;
}

HWND CAutoScrollWnd::Create( HINSTANCE hInstance, HWND hwndParent, bool bVertical, bool bHorizontal, const CMyPoint& point, CEditView* view )
{
	UNREFERENCED_PARAMETER(bVertical);
	UNREFERENCED_PARAMETER(bHorizontal);

	LPCWSTR pszClassName = m_ClassName.c_str();

	m_cView = view;

	/* ウィンドウクラス作成 */
	const auto atom = RegisterClassW(HBRUSH(COLOR_3DFACE + 1), ::LoadCursorW(hInstance, MAKEINTRESOURCE(m_CursorId)));

	LPCWSTR pClassName = m_ClassName.c_str();
	if (atom) {
		pClassName = MAKEINTATOM(atom);
	}

	/* 基底クラスメンバ呼び出し */
	return Base::Create(
		/* 初期化 */
		hwndParent,
		0,
		pClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		point.x-16, // horizontal position of window
		point.y-16, // vertical position of window
		32, // window width
		32, // window height
		nullptr // handle to menu, or child-window identifier
	);
}

void CAutoScrollWnd::Close()
{
	this->DestroyWindow();
}

/*!
 * WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 * メッセージの戻り値はウインドウの作成を続行するかどうかの判断に使われます。
 *
 * @retval true  ウィンドウの作成を続行する
 * @retval false ウィンドウの作成を中止する
 */
bool CAutoScrollWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	if (!Base::OnCreate(hWnd, lpCreateStruct)) {
		return false;
	}

	m_hCenterImg = (HBITMAP)::LoadImageW(lpCreateStruct->hInstance, MAKEINTRESOURCE(m_BitMapId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (!m_hCenterImg) {
		return false;
	}

	return true;
}

/*!
 * @brief WM_DESTROYハンドラ
 *
 * WM_DESTROYはDestroyWindow関数によるウインドウ破棄中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CAutoScrollWnd::OnDestroy(HWND hWnd)
{
	//背景ビットマップを削除する
	m_hCenterImg = nullptr;

	Base::OnDestroy(hWnd);
}

/*!
 * @brief WM_PAINTハンドラ
 *
 * WM_PAINTはウィンドウの描画中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 * @note windowsx.h の定義が微妙なので独自に定義
 */
void CAutoScrollWnd::OnPaint(HWND hWnd, PAINTSTRUCT& ps)
{
	UNREFERENCED_PARAMETER(hWnd);

	HDC hdc = ps.hdc;

	using MemDcHolder = cxx::ResourceHolder<&::DeleteDC>;
	MemDcHolder hCompDc{ ::CreateCompatibleDC(hdc) };

	using SelectionHolder = cxx::ResourceHolder<&::SelectObject>;
	SelectionHolder hBitmap_Old{ hCompDc };
	hBitmap_Old = ::SelectObject(hCompDc, m_hCenterImg);

	::BitBlt(hdc, 0, 0, 32, 32, hCompDc, 0, 0, SRCCOPY);
}

void CAutoScrollWnd::OnLButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(fDoubleClick);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(keyFlags);

	if (!hWnd) return;

	if( m_cView->m_nAutoScrollMode ){
		m_cView->AutoScrollExit();
	}
}

void CAutoScrollWnd::OnRButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(fDoubleClick);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(keyFlags);

	if( m_cView->m_nAutoScrollMode ){
		m_cView->AutoScrollExit();
	}
}

void CAutoScrollWnd::OnMButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(fDoubleClick);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(keyFlags);

	if( m_cView->m_nAutoScrollMode ){
		m_cView->AutoScrollExit();
	}
}
