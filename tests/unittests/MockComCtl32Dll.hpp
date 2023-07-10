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

#include <gmock/gmock.h>

#include "apimodule/ComCtl32Dll.hpp"

struct MockComCtl32Dll : public ComCtl32Dll
{
	~MockComCtl32Dll() override = default;

	MOCK_CONST_METHOD4(GetWindowSubclass, BOOL(
		_In_ HWND hWnd,
		_In_ SUBCLASSPROC pfnSubclass,
		_In_ UINT_PTR uIdSubclass,
		_Out_opt_ DWORD_PTR* pdwRefData));

	MOCK_CONST_METHOD3(RemoveWindowSubclass, BOOL(
		_In_ HWND hWnd,
		_In_ SUBCLASSPROC pfnSubclass,
		_In_ UINT_PTR uIdSubclass));

	MOCK_CONST_METHOD4(SetWindowSubclass, BOOL(
		_In_ HWND hWnd,
		_In_ SUBCLASSPROC pfnSubclass,
		_In_ UINT_PTR uIdSubclass,
		_In_ DWORD_PTR dwRefData));
};
