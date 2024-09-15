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
#pragma once

#include "apiwrap/window/CGenericWnd.hpp"

namespace apiwrap::window
{

/*!
 * カスタムウインドウ
 * 
 * 既存ウインドウ独自定義のウインドウクラスです。
 */
class CCustomizedWnd : public CGenericWnd {
private:
    using Me = CCustomizedWnd;

public:
    WNDPROC m_WndProc = nullptr;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	CCustomizedWnd() = default;

	virtual bool    Attach(HWND hWnd);
	virtual void    Detach(HWND hWnd);

    LRESULT DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const override;
};

extern template
CCustomizedWnd* CGenericWnd::FromHwnd<CCustomizedWnd>(HWND hWnd);

} // end of namespace apiwrap::window
