/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

enum EFunctionCode;
enum EMainMenuType;

/*!
 * メニュー項目構造体
 */
struct SMenuItem {
	static inline std::map<std::wstring, EFunctionCode, std::less<>> topMenuItems{};
	static inline std::map<std::wstring, EFunctionCode, std::less<>> popupMenuItems{};
	static std::vector<SMenuItem> LoadFromResource(WORD resourceId);
	static std::vector<SMenuItem> LoadMainMenuFromResource(WORD resourceId);
	static std::vector<SMenuItem> LoadCustomMenuFromResource(WORD resourceId);

	EFunctionCode	m_eFuncCode;			//!< 機能コード
	int				m_nLevel;				//!< メニュー階層。トップ、メニューバー項目なら0。
	char			m_chAccessKey = '\0';	//!< メニュー項目のアクセスキー。\0は「なし」。

	SMenuItem(
		int nLevel,
		int nFuncCode,
		char accessKey = '\0'
	) noexcept;

	EMainMenuType GetType() const noexcept;
};
