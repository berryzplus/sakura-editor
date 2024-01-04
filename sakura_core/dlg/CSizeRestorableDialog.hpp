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

#include "dlg/CDialog.h"

 /*!
  * 可変ダイアログの基底クラス
  *
  * 表示位置とサイズを復元する機能を付加する。
  */
class CSizeRestorableDialog : public CSakuraDialog
{
protected:
	POINT   m_ptDefaultSize = { -1, -1 };

public:
	explicit CSizeRestorableDialog(WORD idDialog_, const ShareDataAccessor& ShareDataAccessor_, const User32Dll& User32Dll_ = ::GetUser32Dll()) noexcept;
	~CSizeRestorableDialog() override = default;

protected:
	INT_PTR DispatchDlgEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	INT_PTR DispatchEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	BOOL    OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL    OnDlgDestroy(HWND hDlg) override;

	virtual void    OnGetMinMaxInfo(HWND hDlg, _In_ LPMINMAXINFO lpMinMaxInfo);
};
