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

#include "apimodule/User32Dll.hpp"
#include "apiwrap/apiwrap.hpp"
#include "window/CGenericWnd.hpp"

/*!
 * サブクラスウインドウ
 * 
 * WNDPROCを差し替えて、メッセージをフックするウインドウクラスです。
 * 。
 */
class CSubClassedWnd : public CGenericWnd, public User32DllClient
{
private:
    using Me = CSubClassedWnd;

	static constexpr auto& PROP_NAME = L"sakura::CSubClassedWnd";

	WNDPROC _pfnOriginalProc = nullptr;

public:
	using User32DllClient::User32DllClient;

	void Attach(HWND hWnd);
	void Detach(HWND hWnd);

protected:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    LRESULT DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const override;

	virtual void    OnDestroy(HWND hWnd);

private:
	WNDPROC SetWindowProc(_In_ HWND hWnd, _In_ const WNDPROC& pfnWndProc) const;
};

template<class T, int index = 0>
struct TSubClassedChildWnd : public CSubClassedWnd
{
	T* _pThis;

	explicit TSubClassedChildWnd(T* pThis_, const User32Dll& User32Dll_)
		: CSubClassedWnd(User32Dll_)
		, _pThis(pThis_)
	{
	}
};

template<class T, int index = 0>
struct TSubClassedWnd : public TSubClassedChildWnd<T, index>
{
	using TSubClassedChildWnd<T, index>::TSubClassedChildWnd;

	LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};
