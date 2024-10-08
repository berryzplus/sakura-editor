﻿/*!	@file
	@brief プロファイルマネージャ

	@author Moca
	@date 2013.12.31
*/
/*
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_
#define SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_
#pragma once

#include "dlg/CDialog.h"
#include "_main/CCommandLine.h"
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

struct SProfileSettings
{
	WCHAR m_szDllLanguage[_MAX_PATH];
	int	m_nDefaultIndex;
	std::vector<std::wstring> m_vProfList;
	bool m_bDefaultSelect;
};

class CDlgProfileMgr final : public CDialog
{
public:
	//! コマンドラインだけでプロファイルが確定するか調べる
	static bool TrySelectProfile(std::wstring& strProfileName, bool hasProfileName = false, bool showProfileMgr = false) noexcept;

	/*
	||  Constructors
	*/
	CDlgProfileMgr();
	/*
	||  Attributes & Operations
	*/
	int		DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

protected:

	BOOL	OnBnClicked(int wID) override;
	INT_PTR	DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;

	void	SetData() override;	/* ダイアログデータの設定 */
	void	SetData(int nSelIndex);	/* ダイアログデータの設定 */
	int		GetData() override;	/* ダイアログデータの取得 */
	int		GetData(bool bStart);	/* ダイアログデータの取得 */
	LPVOID	GetHelpIdTable(void) override;

	void	UpdateIni();
	void	CreateProf();
	void	DeleteProf();
	void	RenameProf();
	void	SetDefaultProf(int index);
	void	ClearDefaultProf();
public:
	std::wstring m_strProfileName;

	static bool ReadProfSettings(SProfileSettings& settings);
	static bool WriteProfSettings(SProfileSettings& settings);
};

std::filesystem::path GetProfileMgrFileName();
std::filesystem::path GetProfileDirectory(const std::wstring& name);

[[nodiscard]] std::wstring GetProfileMgrFileName(const std::wstring_view& name);

#endif /* SAKURA_CDLGPROFILEMGR_E77A329C_4D06_436A_84E3_01B4D8F34A9A_H_ */
