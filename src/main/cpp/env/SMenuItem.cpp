/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "env/SMenuItem.hpp"

#include "env/CommonSetting.h"

#include "Funccode_enum.h"
#include "String_define.h"

const std::set<int> topFuncCodes = {
	F_FILE_TOPMENU,
	F_EDIT_TOPMENU,
	F_CONVERT_TOPMENU,
	F_SEARCH_TOPMENU,
	F_TOOL_TOPMENU,
	F_OPTION_TOPMENU,
	F_WINDOW_TOPMENU,
	F_HELP_TOPMENU,
};

const std::set<int> popupFuncCodes = {
	F_TEXTWRAPMETHOD,
	F_FILE_REOPEN_SUBMENU,
	F_FILE_RCNTFILE_SUBMENU,
	F_FILE_RCNTFLDR_SUBMENU,
	F_EDIT_INS_SUBMENU,
	F_EDIT_HLV_SUBMENU,
	F_EDIT_MOV_SUBMENU,
	F_EDIT_SEL_SUBMENU,
	F_EDIT_BOX_SEL_SUBMENU,
	F_EDIT_COS_SUBMENU,
	F_CONV_ENCODE_SUBMENU,
	F_ISEARCH_SUBMENU,
	F_BOOKMARK_SUBMENU,
	F_EXECKEYMACRO_REGD,
	F_TOOL_CUSTOM_SUBMENU,
	F_CHGMOD_EOL_SUBMENU,
	F_TAB_MANIP_SUBMENU,
	F_WINDOW_LIST_SUBMENU,
};

const std::set<int> specialFuncCodes = {
	F_FILE_USED_RECENTLY,
	F_FOLDER_USED_RECENTLY,
	F_USERMACRO_LIST,
	F_PLUGIN_LIST,
	F_CUSTMENU_LIST,
	F_WINDOW_LIST,
};

/*!
 * コンストラクタ
 */
SMenuItem::SMenuItem(
	int nLevel,
	int nFuncCode,
	char accessKey
) noexcept
	: m_eFuncCode(EFunctionCode(nFuncCode))
	, m_nLevel(nLevel)
	, m_chAccessKey(accessKey)
{
}

/*!
 * カスタムメニュー用コンストラクタ
 */
SMenuItem::SMenuItem(
	int nFuncCode,
	char accessKey
) noexcept
	: SMenuItem(0, nFuncCode, accessKey)
{
}

EMainMenuType SMenuItem::GetType() const noexcept
{
	// セパレーター
	if (F_SEPARATOR == m_eFuncCode) return T_SEPARATOR;

	// 特殊メニュー
	if (const auto found = specialFuncCodes.find(m_eFuncCode); found != specialFuncCodes.cend()) return T_SPECIAL;

	// 最上位ポップアップメニュー
	if (const auto found = topFuncCodes.find(m_eFuncCode); found != topFuncCodes.cend()) return T_NODE;

	// ポップアップメニュー
	if (const auto found = popupFuncCodes.find(m_eFuncCode); found != popupFuncCodes.cend()) return T_NODE;

	return T_LEAF;
}
