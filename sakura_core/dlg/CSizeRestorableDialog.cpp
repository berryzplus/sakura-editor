/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include "dlg/CSizeRestorableDialog.hpp"

/*!
 * コンストラクタ
 */
CSizeRestorableDialog::CSizeRestorableDialog(WORD idDialog_, std::shared_ptr<ShareDataAccessor> ShareDataAccessor_, std::shared_ptr<User32Dll> User32Dll_) noexcept
	: CSakuraDialog(idDialog_, std::move(ShareDataAccessor_), std::move(User32Dll_))
{
}

/*!
 * ダイアログのメッセージ配送
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
INT_PTR CSizeRestorableDialog::DispatchDlgEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 既存コード互換のために旧関数を呼び出す。
	return DispatchEvent(hDlg, uMsg, wParam, lParam);
}

/*!
 * ダイアログのメッセージ配送
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
INT_PTR CSizeRestorableDialog::DispatchEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// WM_WINDOWPOSCHANGEDが来た場合、個別に処理する
	if (uMsg == WM_WINDOWPOSCHANGED)
	{
		if (const auto lpwpos = std::bit_cast<LPWINDOWPOS>(lParam))
		{
			m_xPos    = lpwpos->x;
			m_yPos    = lpwpos->y;
			m_nWidth  = lpwpos->cx;
			m_nHeight = lpwpos->cy;
		}
	}

	// WM_GETMINMAXINFOが来た場合、個別に処理する
	if (uMsg == WM_GETMINMAXINFO)
	{
		if (const auto lpMinMaxInfo = std::bit_cast<LPMINMAXINFO>(lParam))
		{
			// 戻り値は無視する
			HANDLE_WM_GETMINMAXINFO(hDlg, wParam, lParam, OnGetMinMaxInfo);
		}

		return FALSE;
	}

	// WM_INITDIALOGの戻り値は他と意味が異なるので個別に処理する
	if (uMsg == WM_INITDIALOG)
	{
		RECT rc = {};
		GetClientRect(hDlg, &rc);
		m_ptDefaultSize.x = rc.right - rc.left;
		m_ptDefaultSize.y = rc.bottom - rc.top;

		const auto ret = HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, OnDlgInitDialog);

		SetDialogPosSize();

		m_bInited = TRUE;

		return ret;
	}

	// 基底クラスのハンドラを呼び出す。
	return __super::DispatchEvent(hDlg, uMsg, wParam, lParam);
}

/*!
 * WM_INITDIALOGハンドラ
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] wParam フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE  フォーカスを設定する
 * @retval FALSE フォーカスを設定しない
 */
BOOL CSizeRestorableDialog::OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	// 基底クラスのハンドラを呼び出す。
	const auto ret = __super::OnInitDialog(hDlg, wParam, lParam);

	CreateSizeBox();

	return ret;
}

/*!
 * WM_DESTROYハンドラ
 *
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
BOOL CSizeRestorableDialog::OnDlgDestroy(HWND hDlg)
{
	// 戻り値は無視する
	OnDestroy();

	if (const auto hWndSizeGrip = m_hwndSizeBox) {
		DestroyWindow(hWndSizeGrip);
		m_hwndSizeBox = NULL;
	}

	return TRUE;
}

/*!
 * WM_GETMINMAXINFOハンドラ
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [out] lpMinMaxInfo サイズ情報
 */
void CSizeRestorableDialog::OnGetMinMaxInfo(HWND hDlg, _In_ LPMINMAXINFO lpMinMaxInfo)
{
	UNREFERENCED_PARAMETER(hDlg);
	UNREFERENCED_PARAMETER(lpMinMaxInfo);
}
