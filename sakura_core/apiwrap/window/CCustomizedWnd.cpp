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
#include "apiwrap/window/CCustomizedWnd.hpp"

namespace apiwrap::window
{

template<>
CCustomizedWnd* CGenericWnd::FromHwnd<CCustomizedWnd>(HWND hWnd)
{
    return static_cast<CCustomizedWnd*>((CGenericWnd*)::GetPropW(hWnd, L"CCustomizedWnd"));
}

/*!
 * WndProc(カスタムウインドウのメッセージ配送)
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CALLBACK CCustomizedWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // SetWindowLongPtr/GetWindowLongPtrの引数にNULLはマズい
    if (!hWnd || !::IsWindow(hWnd))
    {
        return 0L;
    }

    // ウインドウプロパティに設定されたインスタンスを取り出す
	auto pcWnd = CGenericWnd::FromHwnd<CCustomizedWnd>(hWnd);
	if (pcWnd && pcWnd->m_WndProc)
    {
		// インスタンスのメッセージ配送を呼び出す
		const auto ret = pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam);

		// WM_DESTROYが来たら紐付けを解除する
		if (WM_DESTROY == uMsg)
		{
			pcWnd->Detach(hWnd);
		}

		return ret;
	}

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

bool CCustomizedWnd::Attach(HWND hWnd)
{
    if (!hWnd || !IsWindow(hWnd) || m_WndProc) {
        return false;
    }

    m_hWnd    = hWnd;
	m_WndProc = WNDPROC(SetWindowLongPtrW(hWnd, GWLP_WNDPROC, LONG_PTR(&WndProc)));

    SetPropW(hWnd, L"CCustomizedWnd", this);

	return m_WndProc;
}

void CCustomizedWnd::Detach(HWND hWnd)
{
    if (!hWnd || hWnd != m_hWnd) {
		return;
    }

	SetPropW(hWnd, L"CCustomizedWnd", nullptr);

	m_WndProc = nullptr;
	m_hWnd    = nullptr;
}

/*!
 * デフォルトのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CCustomizedWnd::DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
    return ::CallWindowProcW(m_WndProc, hWnd, uMsg, wParam, lParam);
}

} // end of namespace apiwrap::window
