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
#include "window/CSubClassedWnd.hpp"

/*!
 * WndProc(カスタムウインドウのメッセージ配送)
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CALLBACK CSubClassedWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// SetWindowLongPtr/GetWindowLongPtrの引数にNULLはマズい
	if (!hWnd)
	{
		return 0L;
	}

	// GetPropでインスタンスを取り出し、処理させる
	auto pcWnd = (Me*)::GetPropW(hWnd, PROP_NAME);
	if (!pcWnd)
	{
		return 0L;
	}

	const auto ret = pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam);

	// WM_DESTROYが来たらウインドウハンドルとインスタンスの紐付けを解除する
	if (uMsg == WM_DESTROY)
	{
		pcWnd->Detach(hWnd);

		pcWnd->m_hWnd = nullptr;

		const auto pfnOriginalProc = WNDPROC(::GetWindowLongPtrW(hWnd, GWLP_WNDPROC));

		return ::CallWindowProcW(pfnOriginalProc, hWnd, uMsg, wParam, lParam);
	}

	return ret;
}

void CSubClassedWnd::Attach(HWND hWnd)
{
	m_hWnd = hWnd;

	GetUser32Dll().SetPropW(hWnd, PROP_NAME, HANDLE(this));

	_pfnOriginalProc = SetWindowProc(hWnd, &WndProc);
}

void CSubClassedWnd::Detach(HWND hWnd)
{
	if (_pfnOriginalProc)
	{
		SetWindowProc(hWnd, _pfnOriginalProc);

		_pfnOriginalProc = nullptr;
	}

	if (GetUser32Dll().GetPropW(hWnd, PROP_NAME))
	{
		GetUser32Dll().RemovePropW(hWnd, PROP_NAME);
	}
}

/*!
 * カスタムウインドウのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CSubClassedWnd::DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY) {
		return HANDLE_WM_DESTROY(hWnd, wParam, lParam, OnDestroy);
    }

	return __super::DispatchEvent(hWnd, uMsg, wParam, lParam);
}

/*!
 * デフォルトメッセージハンドラ
 *
 * User32.dllのCallWindowProcWに処理を委譲します。
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CSubClassedWnd::DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
	return GetUser32Dll().CallWindowProcW(_pfnOriginalProc, hWnd, uMsg, wParam, lParam);
}

/*!
 * WM_DESTROYハンドラ
 */
void CSubClassedWnd::OnDestroy(HWND hWnd)
{
	Detach(hWnd);
}

/*!
 * WNDPROCの差し替え
 */
WNDPROC CSubClassedWnd::SetWindowProc(_In_ HWND hWnd, _In_ const WNDPROC& pfnWndProc) const
{
	return WNDPROC(GetUser32Dll().SetWindowLongPtrW(hWnd, GWLP_WNDPROC, LONG_PTR(pfnWndProc)));
}
