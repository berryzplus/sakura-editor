﻿/*! @file */
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

#include "apimodule/User32Dll.hpp"

#include <gmock/gmock.h>

struct MockUser32Dll : public User32Dll
{
	MOCK_CONST_METHOD5_T(CreateDialogIndirectParamW, HWND(
		_In_opt_ HINSTANCE hInstance,
		_In_ LPCDLGTEMPLATEW lpTemplate,
		_In_opt_ HWND hWndParent,
		_In_opt_ DLGPROC lpDialogFunc,
		_In_ LPARAM dwInitParam));

	MOCK_CONST_METHOD5_T(CreateDialogParamW, HWND(
		_In_opt_ HINSTANCE hInstance,
		_In_ LPCWSTR       lpTemplateName,
		_In_opt_ HWND      hWndParent,
		_In_opt_ DLGPROC   lpDialogFunc,
		_In_ LPARAM        dwInitParam));

	MOCK_CONST_METHOD5_T(DialogBoxParamW, INT_PTR(
		_In_opt_ HINSTANCE hInstance,
		_In_ LPCWSTR       lpTemplateName,
		_In_opt_ HWND      hWndParent,
		_In_opt_ DLGPROC   lpDialogFunc,
		_In_ LPARAM        dwInitParam));

	MOCK_CONST_METHOD2(GetDlgItem, HWND(
		_In_opt_ HWND hDlg,
		_In_ int nIDDlgItem));

	MOCK_CONST_METHOD3(GetWindowTextW, int(
		_In_ HWND hWnd,
		_Out_writes_(nMaxCount) LPWSTR lpString,
		_In_ int nMaxCount));

	MOCK_CONST_METHOD1(GetWindowTextLengthW, int(
		_In_ HWND hWnd));

	MOCK_CONST_METHOD4(SendMessageW, LRESULT(
		_In_ HWND hWnd,
		_In_ UINT Msg,
		_Pre_maybenull_ _Post_valid_ WPARAM wParam,
		_Pre_maybenull_ _Post_valid_ LPARAM lParam));

	MOCK_CONST_METHOD2(SetWindowTextW, bool(
		_In_ HWND hWnd,
		_In_opt_ LPCWSTR lpString));

	MOCK_CONST_METHOD2_T(ShowWindow, bool(
		_In_ HWND hWnd,
		_In_ int nCmdShow));
};