/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "pch.h"
#include "util/MessageBoxF.h"

#include "CSelectLang.h"
#include "String_define.h"

#include "testing/MessageBoxHook.hpp"

namespace message_box {

/*!
	MessageBoxHookのテスト 
 */
TEST(MessageBoxHook, test001)
{
	testing::MessageBoxHook hook;

	MessageBoxExW(nullptr, L"2行をマージしました。", L"caption", MB_OK, 1041);

	EXPECT_THAT(hook.back().text, StrEq(L"2行をマージしました。"));
	EXPECT_THAT(hook.back().caption, StrEq(L"caption"));
	EXPECT_THAT(hook.back().uType, MB_OK);
}

struct MessageBoxTest : public ::testing::Test {
	using Base = ::testing::Test;

	static inline HWND hWnd = nullptr;
};

/*!
	MessageBoxFのテスト 
 */
TEST_F(MessageBoxTest, MessageBoxF001)
{
	EXPECT_MSGBOX2(MessageBoxF(hWnd, MB_OK, L"caption", L"%d行をマージしました。", 2), L"2行をマージしました。", L"caption");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ErrorMessage001)
{
	//エラー：赤丸に「×」[OK]
	EXPECT_MSGBOX(ErrorMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ErrorMessage002)
{
	//エラー：赤丸に「×」[OK]
	EXPECT_MSGBOX(TopErrorMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, WarningMessage001)
{
	//警告：三角に「！」[OK]
	EXPECT_MSGBOX(WarningMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, WarningMessage002)
{
	//警告：三角に「！」[OK]
	EXPECT_MSGBOX(TopWarningMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, InfoMessage001)
{
	//情報：青丸に「i」[OK]
	EXPECT_MSGBOX(InfoMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, InfoMessage002)
{
	//情報：青丸に「i」[OK]
	EXPECT_MSGBOX(TopInfoMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ConfirmMessage001)
{
	//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
	EXPECT_MSGBOX(ConfirmMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ConfirmMessage002)
{
	//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
	EXPECT_MSGBOX(TopConfirmMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, Select3Message001)
{
	//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
	EXPECT_MSGBOX(Select3Message(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, Select3Message002)
{
	//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
	EXPECT_MSGBOX(TopSelect3Message(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, OkMessage001)
{
	//その他メッセージ表示用ボックス[OK]
	EXPECT_MSGBOX(OkMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, OkMessage002)
{
	//その他メッセージ表示用ボックス[OK]
	EXPECT_MSGBOX(TopOkMessage(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, CustomMessage001)
{
	//タイプ指定メッセージ表示用ボックス
	EXPECT_MSGBOX(CustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, CustomMessage002)
{
	//タイプ指定メッセージ表示用ボックス
	EXPECT_MSGBOX(TopCustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2), L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, PleaseReportToAuthor001)
{
	//作者に教えて欲しいエラー
	EXPECT_MSGBOX2(PleaseReportToAuthor(hWnd, L"%d行をマージしました。", 2), L"2行をマージしました。", LS(STR_ERR_DLGDOCLMN1));
}

} // namespace message_box
