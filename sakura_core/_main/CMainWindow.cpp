/*! @file */
/*
	Copyright (C) 2024, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */
#include "StdAfx.h"
#include "_main/CMainWindow.hpp"

#include "_main/CNormalProcess.h"

#include "CSelectLang.h"


/*! 共通設定 プロパティシート */
bool CMainWindow::OpenPropertySheet(int nPageNum)
{
	const bool bTrayProc = !getEditorProcess();
	return m_pcPropertyManager->OpenPropertySheet(GetHwnd(), nPageNum, bTrayProc);
}

/*! タイプ別設定 プロパティシート */
bool CMainWindow::OpenPropertySheetTypes(CTypeConfig nSettingType, int nPageNum)
{
	return m_pcPropertyManager->OpenPropertySheetTypes(GetHwnd(), nPageNum, nSettingType);
}

/*!
 * メインウインドウのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CMainWindow::DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
// clang-format off
//	HANDLE_MSG(hWnd, WM_COMMAND,                        OnCommand);
// clang-format on

	case WM_MENUCHAR:       return m_cMenuDrawer.OnMenuChar(hWnd, uMsg, wParam, lParam);
	case WM_DRAWITEM:       return OnDrawItem(hWnd, LPDRAWITEMSTRUCT(lParam));
	case WM_MEASUREITEM:    return OnMeasureItem(hWnd, LPMEASUREITEMSTRUCT(lParam));
	case WM_EXITMENULOOP:   return (OnExitMenuLoop(hWnd, bool(wParam)), 0L);

	default:
		break;
	}
	
	return __super::DispatchEvent(hWnd, uMsg, wParam, lParam);
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
bool CMainWindow::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    if (!__super::OnCreate(hWnd, lpCreateStruct))
    {
        return false;
    }

	m_hIcons.Create(m_hInstance);
	m_cMenuDrawer.Create(CSelectLang::getLangRsrcInstance(), hWnd, &m_hIcons);
	m_pcPropertyManager->Create(hWnd, &m_hIcons, &m_cMenuDrawer );

	return true;
}

/*!
 * WM_DRAWITEMハンドラ
 *
 * windowsx.hの実装では値を返せないので独自に定義
 * 
 * @retval true  このメッセージを処理した
 * @retval false このメッセージを処理しなかった
 */
bool CMainWindow::OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT* lpDrawItem)
{
	if (!lpDrawItem || ODT_MENU != lpDrawItem->CtlType) {
		return false;
	}

	m_cMenuDrawer.DrawItem(lpDrawItem);

	return true;
}

/*!
 * WM_MEASUREITEMハンドラ
 *
 * windowsx.hの実装では値を返せないので独自に定義
 *
 * @retval true  このメッセージを処理した
 * @retval false このメッセージを処理しなかった
 */
bool CMainWindow::OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
	if (!lpMeasureItem || ODT_MENU != lpMeasureItem->CtlType) {
		return false;
	}

	int nItemHeight = 0;
	if (const auto nItemWidth = m_cMenuDrawer.MeasureItem(lpMeasureItem->itemID, &nItemHeight);
			0 < nItemWidth)
	{
		lpMeasureItem->itemWidth  = nItemWidth;
		lpMeasureItem->itemHeight = nItemHeight;
	}

	return true;
}

/*!
 * WM_EXITMENULOOPハンドラ
 *
 * windowsx.hに実装がないので独自に定義
 *
 * DefWindowProc 関数は 0 を返すので「戻り値なし」で定義している。。
 */
void CMainWindow::OnExitMenuLoop(HWND hWnd, bool fShortcut)
{
	m_cMenuDrawer.EndDrawMenu();
}
