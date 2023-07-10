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
#pragma once

#include <Windows.h>
#include <CommCtrl.h>

#include <memory>

/*!
 * ComCtl32.DLLへの依存を抽象化するクラス
 *
 *
 * Win32 API呼出のテストを可能にする
 *
 * * 必要な定義を CommCtrl.h からコピペする
 * * （定義順はアルファベット順、意味的なまとまりは考慮しない。）
 * * 関数定義を virtual const に変える
 * * 関数本体でグローバルの同名関数に処理を委譲させる
 */
struct ComCtl32Dll
{
	virtual ~ComCtl32Dll() = default;

	virtual BOOL GetWindowSubclass(
		_In_ HWND hWnd,
		_In_ SUBCLASSPROC pfnSubclass,
		_In_ UINT_PTR uIdSubclass,
		_Out_opt_ DWORD_PTR* pdwRefData) const
	{
		return ::GetWindowSubclass(hWnd, pfnSubclass, uIdSubclass, pdwRefData);
	}

	virtual BOOL RemoveWindowSubclass(
		_In_ HWND hWnd,
		_In_ SUBCLASSPROC pfnSubclass,
		_In_ UINT_PTR uIdSubclass) const
	{
		return ::RemoveWindowSubclass(hWnd, pfnSubclass, uIdSubclass);
	}

	virtual BOOL SetWindowSubclass(
		_In_ HWND hWnd,
		_In_ SUBCLASSPROC pfnSubclass,
		_In_ UINT_PTR uIdSubclass,
		_In_ DWORD_PTR dwRefData) const
	{
		return ::SetWindowSubclass(hWnd, pfnSubclass, uIdSubclass, dwRefData);
	}
};

class ComCtl32DllClient
{
	const ComCtl32Dll& _ComCtl32Dll;

public:
	explicit ComCtl32DllClient( const ComCtl32Dll& ComCtl32Dll_ )
		: _ComCtl32Dll( ComCtl32Dll_ )
	{
	}
	virtual ~ComCtl32DllClient() = default;

protected:
	const ComCtl32Dll& GetComCtl32Dll() const noexcept
	{
		return _ComCtl32Dll;
	}
};

const ComCtl32Dll& GetComCtl32Dll();
