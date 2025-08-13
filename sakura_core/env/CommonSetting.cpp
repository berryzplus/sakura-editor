/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "env/CommonSetting.h"

#include "env/CShareData.h"
#include "env/CShareData_IO.h"
#include "env/SMenuItem.hpp"
#include "CDataProfile.h"
#include "uiparts/CImageListMgr.h"
#include "util/os.h"
#include "util/window.h"

#include "CSelectLang.h"
#include "sakura_rc.h"
#include "String_define.h"

CommonSetting::CommonSetting(
	const std::filesystem::path& iniFolder
) noexcept
	: m_sMacro(iniFolder)
{
}

CommonSetting_Window::CommonSetting_Window() noexcept
{
	// L"${w?$h$:アウトプット$:${I?$f$n$:$N$n$}$}${U?(更新)$} - $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>"
	wcscpy_s(m_szWindowCaptionActive, LS(STR_ERR_CSHAREDATA17));
	// L"${w?$h$:アウトプット$:$f$n$}${U?(更新)$} - $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>"
	wcscpy_s(m_szWindowCaptionInactive, LS(STR_ERR_CSHAREDATA18));
}

CommonSetting_TabBar::CommonSetting_TabBar() noexcept
{
	auto& lfIconTitle = m_lf;
	auto& nIconPointSize = m_nPointSize;
	SystemParametersInfoW(
		SPI_GETICONTITLELOGFONT,
		sizeof(LOGFONT),
		&lfIconTitle,
		0UL
	);
	nIconPointSize = lfIconTitle.lfHeight >= 0 ? lfIconTitle.lfHeight : DpiPixelsToPoints(-lfIconTitle.lfHeight, 10);

	// L"${w?【Grep】$h$:【アウトプット】$:$f$n$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}"
	wcscpy_s(m_szTabWndCaption, LS(STR_ERR_CSHAREDATA10));
}

CommonSetting_Format::CommonSetting_Format() noexcept
{
	wcscpy_s(m_szMidashiKigou, LS(STR_ERR_CSHAREDATA14));
	wcscpy_s(m_szInyouKigou, L"> ");
	wcscpy_s(m_szDateFormat, LS(STR_ERR_CSHAREDATA15));
	wcscpy_s(m_szTimeFormat, LS(STR_ERR_CSHAREDATA16));
}

/*!
 * @brief 共有メモリ初期化/ポップアップメニュー
 *
 *	ポップアップメニューの初期化処理
 *
 *	@date 2005.01.30 genta CShareData::Init()から分離．
 */
CommonSetting_CustomMenu::CommonSetting_CustomMenu() noexcept
{
	/* カスタムメニュー 規定値 */
	for (int i = 0; i < MAX_CUSTOM_MENU; ++i) {
		m_szCustMenuNameArr[i][0] = '\0';
		m_nCustMenuItemNumArr[i] = 0;
		for (int j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j) {
			m_nCustMenuItemFuncArr[i][j] = F_0;
			m_nCustMenuItemKeyArr [i][j] = '\0';
		}
		m_bCustMenuPopupArr[i] = true;
	}
	m_szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND][0] = '\0';	//@@@ 2003.06.13 MIK
	
	/* 右クリックメニュー */
	const auto editRMenuItems = SMenuItem::LoadCustomMenuFromResource(F_MENU_RBUTTON);
	constexpr int EDIT_RMENU = 0;
	for (size_t i = 0; i < std::size(editRMenuItems); ++i) {
		m_nCustMenuItemFuncArr[EDIT_RMENU][i] = editRMenuItems[i].m_eFuncCode;
		m_nCustMenuItemKeyArr [EDIT_RMENU][i] = editRMenuItems[i].m_chAccessKey;
	}
	m_nCustMenuItemNumArr[EDIT_RMENU] = int(std::size(editRMenuItems));

	/* カスタムメニュー１ */
	const auto custumMenu1Items = SMenuItem::LoadCustomMenuFromResource(F_CUSTMENU_1);
	constexpr int CUST_MENU = 1;
	for (size_t i = 0; i < std::size(custumMenu1Items); ++i) {
		m_nCustMenuItemFuncArr[CUST_MENU][i] = custumMenu1Items[i].m_eFuncCode;
		m_nCustMenuItemKeyArr [CUST_MENU][i] = custumMenu1Items[i].m_chAccessKey;
	}
	m_nCustMenuItemNumArr[CUST_MENU] = int(std::size(custumMenu1Items));

	/* タブメニュー */
	const auto tabMenuItems = SMenuItem::LoadCustomMenuFromResource(F_CUSTMENU_24);
	constexpr int TAB_RMENU = CUSTMENU_INDEX_FOR_TABWND;
	for (size_t i = 0; i < std::size(tabMenuItems); ++i) {
		m_nCustMenuItemFuncArr[TAB_RMENU][i] = tabMenuItems[i].m_eFuncCode;
		m_nCustMenuItemKeyArr [TAB_RMENU][i] = tabMenuItems[i].m_chAccessKey;
	}
	m_nCustMenuItemNumArr[TAB_RMENU] = int(std::size(tabMenuItems));
}

/*static*/ std::vector<int> CommonSetting_ToolBar::GetDefaultTools() noexcept
{
	constexpr std::array defaultToolFuncCodes = {
		F_FILENEW,				//新規作成
		F_FILEOPEN_DROPDOWN,	//ファイルを開く(DropDown)
		F_FILESAVE,				//上書き保存
		F_FILESAVEAS_DIALOG,	//名前を付けて保存
		F_SEPARATOR,
		F_UNDO,					//元に戻す(Undo)
		F_REDO,					//やり直し(Redo)
		F_SEPARATOR,
		F_JUMPHIST_PREV,		//移動履歴: 前へ
		F_JUMPHIST_NEXT,		//移動履歴: 次へ
		F_SEPARATOR,
		F_SEARCH_DIALOG,		//検索
		F_SEARCH_NEXT,			//次を検索
		F_SEARCH_PREV,			//前を検索
		F_REPLACE_DIALOG,		//置換
		F_SEARCH_CLEARMARK,		//検索マークのクリア
		F_GREP_DIALOG,			//Grep
		F_SEPARATOR,
		F_OUTLINE,				//アウトライン解析
		F_SEPARATOR,
		F_TYPE_LIST,			//タイプ別設定一覧
		F_OPTION_TYPE,			//タイプ別設定
		F_OPTION,				//共通設定
		F_SEPARATOR,
		F_MENU_ALLFUNC,			//コマンド一覧
	};

	const auto buttonIds = CImageListMgr::GetFuncIcons();

	std::vector<int> defaultTools;
	std::ranges::transform(
		defaultToolFuncCodes,
		std::back_inserter(defaultTools),
		[&buttonIds](const auto funcCode) {
			return buttonIds.at(funcCode);
		}
	);

	return defaultTools;
}

/*!
 * @brief 共有メモリ初期化/ツールバー
 *
 *	ツールバー関連の初期化処理
 *
 *	@author genta
 *	@date 2005.01.30 genta CShareData::Init()から分離．
 *		一つずつ設定しないで一気にデータ転送するように．
 */
CommonSetting_ToolBar::CommonSetting_ToolBar() noexcept
{
	// デフォルトの定義を読み込む
	const auto defaultTools = GetDefaultTools();

	// ツールバーの設定
	m_nToolBarButtonNum = int(std::size(defaultTools));
	std::ranges::copy(defaultTools, m_nToolBarButtonIdxArr);

	m_bToolBarIsFlat = !IsVisualStyle();
}

CommonSetting_Helper::CommonSetting_Helper() noexcept
{
	auto& lfIconTitle = m_lf;
	auto& nIconPointSize = m_nPointSize;
	SystemParametersInfoW(
		SPI_GETICONTITLELOGFONT,
		sizeof(LOGFONT),
		&lfIconTitle,
		0UL
	);
	nIconPointSize = lfIconTitle.lfHeight >= 0 ? lfIconTitle.lfHeight : DpiPixelsToPoints(-lfIconTitle.lfHeight, 10);
}

CommonSetting_Macro::CommonSetting_Macro(
	const std::filesystem::path& iniFolder
) noexcept
	: m_szMACROFOLDER(iniFolder)
{
}

/* static */ std::vector<std::pair<std::wstring, std::wstring>> CommonSetting_FileName::GetDefaultConversion() noexcept
{
	return {{
		{ LR"(%DeskTop%\)",           LS(STR_TRANSNAME_DESKTOP) },
		{ LR"(%Personal%\)",          LS(STR_TRANSNAME_MYDOC) },
		{ LR"(%Cache%\Content.IE5\)", LS(STR_TRANSNAME_IE) },
		{ LR"(%TEMP%\)",              LS(STR_TRANSNAME_TEMP) },
		{ LR"(%Common DeskTop%\)",    LS(STR_TRANSNAME_COMDESKTOP) },
		{ LR"(%Common Documents%\)",  LS(STR_TRANSNAME_COMDOC) },
		{ LR"(%AppData%\)",           LS(STR_TRANSNAME_APPDATA) }
	}};
}

CommonSetting_FileName::CommonSetting_FileName() noexcept
{
	const auto defaultConversions = GetDefaultConversion();

	m_nTransformFileNameArrNum = int(std::size(defaultConversions));

	for (size_t i = 0; i < defaultConversions.size(); ++i) {
		wcscpy_s(m_szTransformFileNameFrom[i], defaultConversions[i].first.c_str());
		wcscpy_s(m_szTransformFileNameTo[i], defaultConversions[i].second.c_str());
	}
}

CommonSetting_View::CommonSetting_View() noexcept
{
	auto& lf = m_lf;
	lf.lfHeight			= DpiPointsToPixels(-10);
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;

	StringBufferW(lf.lfFaceName) = L"ＭＳ ゴシック";
}

CommonSetting_MainMenu::CommonSetting_MainMenu() noexcept
{
	const auto menuItems = SMenuItem::LoadMainMenuFromResource(IDR_MAINMENU);

	m_nMainMenuNum = int(std::size(menuItems));
	for (size_t i = 0; i < m_nMainMenuNum; ++i) {
		m_cMainMenuTbl[i].m_nLevel   = menuItems[i].m_nLevel;
		m_cMainMenuTbl[i].m_nType    = menuItems[i].GetType();
		m_cMainMenuTbl[i].m_nFunc    = menuItems[i].m_eFuncCode;
		m_cMainMenuTbl[i].m_sKey[0]  = menuItems[i].m_chAccessKey;
		m_cMainMenuTbl[i].m_sKey[1]  = L'\0';
		m_cMainMenuTbl[i].m_sName[0] = L'\0';
	}

	// m_nLevel==0の要素のインデックスをm_nMenuTopIdxに格納
	size_t topIdxCount = 0;
	for (size_t i = 0; i < menuItems.size() && topIdxCount < std::size(m_nMenuTopIdx); ++i) {
		if (menuItems[i].m_nLevel == 0) {
			m_nMenuTopIdx[topIdxCount++] = static_cast<int>(i);
		}
	}
	for (size_t i = topIdxCount; i < std::size(m_nMenuTopIdx); ++i) {
		m_nMenuTopIdx[i] = -1;
	}
}
