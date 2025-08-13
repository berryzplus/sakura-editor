/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "env/SMenuItem.hpp"

#include "basis/message_error.hpp"
#include "cxx_util/ResourceHolder.hpp"

#include "Funccode_enum.h"
#include "tests1_rc.h"
#include "sakura_rc.h"

namespace share_data {

using MenuHolder = cxx_util::ResourceHolder<HMENU, &DestroyMenu>;

/*!
	SMenuItemのテスト

	指定した通りに初期化されること。
 */
TEST(SMenuItem, SMenuItem)
{
	// ARRANGE
	SMenuItem item = { 2, F_FILENEW, 'X' };

	// ASSERT
	EXPECT_THAT(item.m_nLevel, 2);
	EXPECT_THAT(item.m_eFuncCode, F_FILENEW);
	EXPECT_THAT(item.m_chAccessKey, 'X');
}

/*!
	SMenuItemのテスト：存在しないリソースIDを指定した場合。
 */
TEST(SMenuItem, LoadFromResource_100)
{
	// 存在しないリソースIDを指定したら空が返る
	constexpr auto IDR_NOT_FOUND = 65535;
	const auto menuItems = SMenuItem::LoadFromResource(IDR_NOT_FOUND);
	EXPECT_TRUE(menuItems.empty());
}

/*!
	SMenuItemのテスト：不明なトップ項目
 */
TEST(SMenuItem, LoadFromResource_101)
{
	const auto menuItems = SMenuItem::LoadFromResource(IDR_BAD_MAINMENU1);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

/*!
	SMenuItemのテスト：トップ項目のアクセスキーがない
 */
TEST(SMenuItem, LoadFromResource_102)
{
	const auto menuItems = SMenuItem::LoadFromResource(IDR_BAD_MAINMENU2);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

/*!
	SMenuItemのテスト：不明なポップアップ項目
 */
TEST(SMenuItem, LoadFromResource_103)
{
	EXPECT_THAT([] { SMenuItem::LoadFromResource(IDR_BAD_MAINMENU3); }, ThrowsMessage<basis::message_error>(StrEq("unknown popup menu \"不明な項目\"")));
}

/*!
	SMenuItemのテスト：ポップアップ項目のアクセスキーがない
 */
TEST(SMenuItem, LoadFromResource_104)
{
	const auto menuItems = SMenuItem::LoadFromResource(IDR_BAD_MAINMENU4);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

/*!
	SMenuItemのテスト：不明なメニュー項目
 */
TEST(SMenuItem, LoadFromResource_105)
{
	const auto menuItems = SMenuItem::LoadFromResource(IDR_BAD_MAINMENU5);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

/*!
	SMenuItemのテスト：メニュー項目のアクセスキーがない
 */
TEST(SMenuItem, LoadFromResource_106)
{
	const auto menuItems = SMenuItem::LoadFromResource(IDR_BAD_MAINMENU6);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

/*!
	SMenuItemのテスト：存在しないリソースIDを指定した場合。
 */
TEST(SMenuItem, LoadMainMenuFromResource_100)
{
	// 存在しないリソースIDを指定したら空が返る
	constexpr auto IDR_NOT_FOUND = 65535;
	const auto menuItems = SMenuItem::LoadMainMenuFromResource(IDR_NOT_FOUND);
	EXPECT_TRUE(menuItems.empty());
}

/*!
	SMenuItemのテスト：不明なトップ項目
 */
TEST(SMenuItem, LoadMainMenuFromResource_101)
{
	EXPECT_THAT([] { SMenuItem::LoadMainMenuFromResource(IDR_BAD_MAINMENU1); }, ThrowsMessage<basis::message_error>(StrEq("bad top menu. index 0")));
}

/*!
	SMenuItemのテスト：トップ項目のアクセスキーがない
 */
TEST(SMenuItem, LoadMainMenuFromResource_102)
{
	EXPECT_THAT([] { SMenuItem::LoadMainMenuFromResource(IDR_BAD_MAINMENU2); }, ThrowsMessage<basis::message_error>(StrEq("bad top menu. index 0")));
}

/*!
	SMenuItemのテスト：不明なポップアップ項目
 */
TEST(SMenuItem, LoadMainMenuFromResource_103)
{
	EXPECT_THAT([] { SMenuItem::LoadMainMenuFromResource(IDR_BAD_MAINMENU3); }, ThrowsMessage<basis::message_error>(StrEq("unknown popup menu \"不明な項目\"")));
}

/*!
	SMenuItemのテスト：ポップアップ項目のアクセスキーがない
 */
TEST(SMenuItem, LoadMainMenuFromResource_104)
{
	const auto menuItems = SMenuItem::LoadFromResource(IDR_BAD_MAINMENU4);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

/*!
	SMenuItemのテスト：不明なメニュー項目
 */
TEST(SMenuItem, LoadMainMenuFromResource_105)
{
	const auto menuItems = SMenuItem::LoadMainMenuFromResource(IDR_BAD_MAINMENU5);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

/*!
	SMenuItemのテスト：メニュー項目のアクセスキーがない
 */
TEST(SMenuItem, LoadMainMenuFromResource_106)
{
	const auto menuItems = SMenuItem::LoadMainMenuFromResource(IDR_BAD_MAINMENU6);
	EXPECT_THAT(menuItems.size(), Gt(1));
}

}

} // namespace share_data
