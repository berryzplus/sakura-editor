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
#include "dlg/CDlgFind.h"

#include "doc/CEditDoc.h"
#include "view/CEditView.h"

#include "MockShareDataAccessor.hpp"

/*!
 * 検索ダイアログ、構築するだけ。
 */
TEST(CDlgFind, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	EXPECT_NO_THROW({ CDlgFind dlg(std::move(pShareDataAccessor)); });
}

/*!
 * 表示テスト
 */
TEST(CDlgFind, SimpleShowDialog)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();

	pDllShareData->m_Common.m_sSearch.m_sSearchOption.bRegularExp = true;

	CEditDoc doc(pShareDataAccessor);
	CEditView view;
	CDlgFind dlg(std::move(pShareDataAccessor));
	const auto hInstance  = static_cast<HINSTANCE>(nullptr);
	const auto hWndParent = static_cast<HWND>(nullptr);
	const auto lParam     = LPARAM(&view);
	const auto hDlg       = dlg.DoModeless(hInstance, hWndParent, lParam);
	EXPECT_NE(nullptr, hDlg);
	dlg.CloseDialog(0);
}
