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

struct CAutoScrollCWnd final : public CAutoScrollWnd
{
	CAutoScrollCWnd() : CAutoScrollWnd(true, true) {}
};

struct CAutoScrollVWnd final : public CAutoScrollWnd
{
	CAutoScrollVWnd() : CAutoScrollWnd(true, false) {}
};

struct CAutoScrollHWnd final : public CAutoScrollWnd
{
	CAutoScrollHWnd() : CAutoScrollWnd(false, false) {}
};

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

HWND CAutoScrollWnd::Open(CEditView* pcEditView, const CMyPoint& pt)
{
	m_cView = pcEditView;

	const auto hInstance = pcEditView->GetAppInstance();
	const auto hWndParent = pcEditView->GetHwnd();

	/* ウィンドウクラス作成 */
	const auto atom = RegisterClassW(HBRUSH(COLOR_3DFACE + 1), ::LoadCursorW(hInstance, MAKEINTRESOURCE(m_CursorId)));

	const auto cxIcon = GetSystemMetrics(SM_CXICON);
	const auto cyIcon = GetSystemMetrics(SM_CYICON);

	CMyRect rc;
	rc.SetPos(pt.x - cxIcon / 2, pt.y - cyIcon / 2);
	rc.SetSize(cxIcon, cyIcon);

	/* 基底クラスメンバ呼び出し */
	return Base::CreateWnd(atom, WS_CHILD | WS_VISIBLE, hWndParent, 0, rc, m_ClassName);
}

/*!
 * @brief オートスクロールウィンドウのメッセージ配送
 *
 * @param hWnd [in] 宛先ウインドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CAutoScrollWnd::DispatchEvent(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (uMsg) {
// clang-format off
	HANDLE_MSG(hWnd, WM_LBUTTONDOWN,					ExitAutoScroll);
	HANDLE_MSG(hWnd, WM_RBUTTONDOWN,					ExitAutoScroll);
	HANDLE_MSG(hWnd, WM_MBUTTONDOWN,					ExitAutoScroll);
// clang-format on

	default:
		break;
	}

	//あとはデフォルトに任せる
	return Base::DispatchEvent(hWnd, uMsg, wParam, lParam);
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

	m_hCenterImg = (HBITMAP)::LoadImageW(GetAppInstance(), MAKEINTRESOURCE(m_BitMapId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (!m_hCenterImg) return false;

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

	MemDcHolder hCompDc{ ::CreateCompatibleDC(hdc) };

	SelectionHolder hBitmapOld{ hCompDc };
	hBitmapOld = ::SelectObject(hCompDc, m_hCenterImg);

	::BitBlt(hdc, 0, 0, 32, 32, hCompDc, 0, 0, SRCCOPY);
}

void CAutoScrollWnd::ExitAutoScroll(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	UNREFERENCED_PARAMETER(fDoubleClick);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(keyFlags);

	if (!hWnd) {
		return;
	}

	if (m_cView && m_cView->m_nAutoScrollMode) {
		m_cView->AutoScrollExit();
	}
}
