/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "env/SMenuItem.hpp"

#include "basis/message_error.hpp"
#include "cxx_util/ResourceHolder.hpp"
#include "env/CommonSetting.h"
#include "CDataProfile.h"
#include "CSelectLang.h"

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

const std::wregex labelPattern(LR"(^(.+?)(?:\(&(.)\))?$)");

/*!
 * 機能コードから、ラベルと機能コードのペアを作る
 */
std::pair<std::wstring, EFunctionCode> makeLabelAndFuncIdPair(const int id)
{
	return std::make_pair(std::wstring(LS(id)), EFunctionCode(id));
}

void ParseMenuItem(
	std::vector<SMenuItem>& menuItems,
	HMENU hMenu,
	int nPos,
	int nLevel,
	StringBufferW szLabel
)
{
	MENUITEMINFO mii = { sizeof(mii), MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_SUBMENU };

	mii.dwTypeData = szLabel;
	mii.cch        = UINT(std::size(szLabel));

	GetMenuItemInfoW(hMenu, nPos, TRUE, &mii);

	auto eFuncCode = EFunctionCode(mii.wID);

	std::wstring label;
	char accessKey = '\0';

	// メニュー区切り線
	if (mii.fType & MF_SEPARATOR) {
		eFuncCode = F_SEPARATOR;
	}
	// パターンマッチでラベルとアクセスキーを取得できる場合
	else if (std::wcmatch matches; std::regex_match(mii.dwTypeData, matches, labelPattern))
	{
		label     = matches[1];
		accessKey = matches[2].matched ? static_cast<char>(*matches[2].first) : 0;

		// サブメニュー
		if (mii.hSubMenu) {
			const auto& popupMenuItems = SMenuItem::popupMenuItems;

			// ポップアップメニュー
			if (const auto foundPopup = popupMenuItems.find(label); foundPopup != popupMenuItems.cend()) {
				eFuncCode = EFunctionCode(foundPopup->second);
			} else {
				throw basis::message_error(std::format(LR"(unknown popup menu "{}")", label));
			}
		}
	}

	menuItems.emplace_back(nLevel, eFuncCode, accessKey);

	if (const auto hSubMenu = mii.hSubMenu) {
		const auto menuItemCount = GetMenuItemCount(hSubMenu);
		for (int nSubPos = 0; nSubPos < menuItemCount; ++nSubPos) {
			ParseMenuItem(menuItems, hSubMenu, nSubPos, nLevel + 1, szLabel);
		}
	}
}

/*!
 * メニューハンドルから読み込む
 */
/* static */ std::vector<SMenuItem> SMenuItem::LoadFromResource(WORD resourceId)
{
	using MenuHolder = cxx_util::ResourceHolder<HMENU, &DestroyMenu>;
	const MenuHolder hMenu = LoadMenuW(nullptr, MAKEINTRESOURCE(resourceId));

	// メニューリソースが読み込めなかった場合は空の配列を返す
	if (!hMenu) {
		return {};
	}

	// HMENUに含まれるアイテム数を取得
	const auto menuItemCount = GetMenuItemCount(hMenu);

	// 空のメニューは作成できないので考慮不要。
	assert(0 < menuItemCount);

	// 最上位トップメニューのラベルと機能コードのペアを再作成
	topMenuItems.clear();
	std::ranges::transform(topFuncCodes, std::inserter(topMenuItems, topMenuItems.end()), makeLabelAndFuncIdPair);

	// ポップアップメニューのラベルと機能コードのペアを再作成
	popupMenuItems.clear();
	std::ranges::transform(popupFuncCodes, std::inserter(popupMenuItems, popupMenuItems.end()), makeLabelAndFuncIdPair);

	// 戻り値配列を宣言する
	std::vector<SMenuItem> menuItems;

	// メニュー項目数だけ確保する
	menuItems.reserve(menuItemCount);

	SString<_MAX_PATH> szLabel;

	for (int nPos = 0; nPos < menuItemCount; ++nPos) {
		MENUITEMINFO mii = { sizeof(mii), MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_SUBMENU };

		mii.dwTypeData = szLabel;
		mii.cch        = UINT(std::size(szLabel));

		GetMenuItemInfoW(hMenu, nPos, TRUE, &mii);

		auto eFuncCode = F_0;

		std::wstring label;
		char accessKey = '\0';

		// パターンマッチでラベルとアクセスキーを取得
		if (std::wcmatch matches; std::regex_match(mii.dwTypeData, matches, labelPattern)) {
			label     = matches[1];
			accessKey = matches[2].matched ? static_cast<char>(*matches[2].first) : 0;
		}

		if (const auto found = topMenuItems.find(label); found != topMenuItems.cend()) {
			eFuncCode = EFunctionCode(found->second);
		}

		menuItems.emplace_back(0, eFuncCode, accessKey);

		if (const auto hSubMenu = mii.hSubMenu) {
			const auto subMenuItemCount = GetMenuItemCount(hSubMenu);
			for (int nSubPos = 0; nSubPos < subMenuItemCount; ++nSubPos) {
				ParseMenuItem(menuItems, hSubMenu, nSubPos, 1, StringBufferW(szLabel));
			}
		}
	}

	return menuItems;
}

/*!
 * メニューハンドルからメインメニューを読み込む
 */
/* static */ std::vector<SMenuItem> SMenuItem::LoadMainMenuFromResource(WORD resourceId)
{
	// 戻り値配列を宣言する
	auto menuItems = LoadFromResource(resourceId);

	// メニューリソースが読み込めなかった場合は空の配列を返す
	if (menuItems.empty()) {
		return {};
	}

	// メインメニューの仕様の違いを吸収するために、セパレーターをF_0に変換する
	for (size_t i = 0; i < menuItems.size(); ++i) {
		const auto& menuItem = menuItems[i];
		if (0 < menuItem.m_nLevel) {
			continue;
		}

		if (F_0 == menuItem.m_eFuncCode) {
			throw basis::message_error(std::format(L"bad top menu. index {}", i));
		}

		if ('\0' == menuItem.m_chAccessKey) {
			throw basis::message_error(std::format(L"bad top menu. index {}", i));
		}
	}

	return menuItems;
}

/*!
 * メニューハンドルから読み込む
 */
/* static */ std::vector<SMenuItem> SMenuItem::LoadCustomMenuFromResource(WORD resourceId)
{
	// 戻り値配列を宣言する
	auto menuItems = LoadFromResource(resourceId);

	// メニューリソースが読み込めなかった場合は空の配列を返す
	if (menuItems.empty()) {
		return {};
	}

	// 先頭1要素を削除する
	menuItems.erase(menuItems.begin());

	// カスタムメニューとの仕様の違いを吸収するために、セパレーターをF_0に変換する
	for (auto& menuItem : menuItems) {
		if (F_SEPARATOR == menuItem.m_eFuncCode) {
			menuItem.m_eFuncCode = F_0;
		}
	}

	return menuItems;
}

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
