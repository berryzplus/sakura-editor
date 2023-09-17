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
#include "dlg/CDlgWindowList.h"

#include "MockShareDataAccessor.hpp"

/*!
 * ファイル一覧ダイアログ、構築するだけ。
 */
TEST(CDlgWindowList, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	EXPECT_NO_THROW({ CDlgWindowList dlg(std::move(pShareDataAccessor)); });
}

/*!
 * 表示テスト
 */
TEST(CDlgWindowList, SimpleShowDialog)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CDlgWindowList dlg(std::move(pShareDataAccessor));
	const auto hWndParent = static_cast<HWND>(nullptr);
	const auto hDlg       = dlg.DoModeless(nullptr, hWndParent, IDD_WINLIST, static_cast<LPARAM>(0), SW_SHOW);
	EXPECT_NE(nullptr, hDlg);
	dlg.CloseDialog(0);
}

TEST(CDlgWindowList, ShowDialogWithSizeRestore)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	auto& rect  = pDllShareData->m_Common.m_sOthers.m_rcWindowListDialog;
	rect.left   = 1;
	rect.top    = 2;
	rect.right  = rect.left + 3;
	rect.bottom = rect.top  + 4;
	CDlgWindowList dlg(std::move(pShareDataAccessor));
	const auto hWndParent = static_cast<HWND>(nullptr);
	const auto hDlg       = dlg.DoModeless(nullptr, hWndParent, IDD_WINLIST, static_cast<LPARAM>(0), SW_SHOW);
	EXPECT_NE(nullptr, hDlg);
	dlg.CloseDialog(0);
}
