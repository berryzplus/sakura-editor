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
	EFunctionCode	m_eFuncCode;			//!< 機能コード
	int				m_nLevel;				//!< メニュー階層。トップ、メニューバー項目なら0。
	char			m_chAccessKey = '\0';	//!< メニュー項目のアクセスキー。\0は「なし」。

	SMenuItem(
		int nLevel,
		int nFuncCode,
		char accessKey = '\0'
	) noexcept;

	SMenuItem(
		int nFuncCode,
		char accessKey = '\0'
	) noexcept;

	EMainMenuType GetType() const noexcept;
};
