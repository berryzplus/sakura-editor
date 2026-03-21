/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "apiwrap/StdControl.h"

#include "cxx/ResourceHolder.hpp"
#include "dlg/CDialog.h"

#include "sakura_rc.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

namespace window {

using WindowHolder = cxx::ResourceHolder<&::DestroyWindow>;

/*!
 * ウインドウ全般を対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, WndTest001)
{
	// パラメーター不正
	EXPECT_THAT(ApiWrap::GetWindowTextW(nullptr), IsFalse());

	const auto expected = L"0123456789012345678901234567890123456789"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_STATIC, std::data(expected), 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// 単純な取得
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(expected));

	// タイトルを空にする
	ApiWrap::SetWindowTextW(hWnd, L""sv);
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), IsFalse());

	CNativeW cmemText;
	ApiWrap::Wnd_SetText(hWnd, nullptr);
	EXPECT_THAT(ApiWrap::Wnd_GetText(hWnd, cmemText), IsFalse());
	ApiWrap::SetWindowTextW(hWnd, L"test"s);
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(L"test"));
	EXPECT_THAT(ApiWrap::Wnd_GetText(hWnd, cmemText), IsTrue());
	EXPECT_THAT(cmemText.GetStringPtr(), StrEq(L"test"));

	// GitHub #1528 の退行防止テストケース。
	// 取得する文字列の長さが basic_string::capacity と同じだった場合に一文字取りこぼしていた。
	std::wstring s(std::size(expected) - 1, L'\0');
	ApiWrap::SetWindowTextW(hWnd, expected);
	EXPECT_THAT(ApiWrap::Wnd_GetText(hWnd, s), IsTrue());
	EXPECT_THAT(s, StrEq(expected));
}

/*!
 * エディットコントロールを対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, EditCtlTest001)
{
	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_EDIT, L"", 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// サイズ20の配列バッファを用意する
	StaticString<20> arrayBuffer{};

	// テキストサイズに制限をかける
	ApiWrap::LimitEditText(hWnd, arrayBuffer);

	// 現在のテキストを取得
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(L""));

	// 制限を無視してテキストを変更（変更できてしまう仕様）
	ApiWrap::SetWindowTextW(hWnd, std::format(L"{:a<20}", L'a'));
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(std::format(L"{:a<20}", L'a')));
}

/*!
 * リストボックスを対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, ListTest001) {
	const auto expected = L"0123456789abcdef"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_LISTBOX, L"", 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// アイテムを追加
	ApiWrap::AddLbItems(hWnd, std::array{ expected, L""s });

	// アイテム数を取得
	EXPECT_THAT(ApiWrap::List_GetCount(hWnd), Eq(2));

	// 初期状態は未選択
	EXPECT_THAT(ApiWrap::List_GetCurSel(hWnd), Eq(LB_ERR));

	// プログラム的にアイテム選択する
	EXPECT_THAT(ApiWrap::List_SetCurSel(hWnd, 1), Eq(1));
	EXPECT_THAT(ApiWrap::List_GetCurSel(hWnd), Eq(1));

	// 指定したアイテムの表示文字列を取得（この機能はあまり重要でない）
	EXPECT_THAT(ApiWrap::GetLbItemText(hWnd, 0), StrEq(expected));
	EXPECT_THAT(ApiWrap::GetLbItemText(hWnd, 1), StrEq(L""));
	EXPECT_THAT(ApiWrap::GetLbItemText(hWnd, 2), IsFalse());

	// 指定したアイテムの表示文字列を取得（この機能はあまり重要でない）
	std::wstring s(std::size(expected), L'\0');
	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 0, std::span{ std::data(s), std::size(expected) - 1 }), LB_ERRSPACE);
	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 0, std::span{ std::data(s), std::size(expected) + 0 }), LB_ERRSPACE);
	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 0, std::span{ std::data(s), std::size(expected) + 1 }), int(std::size(expected)));
	EXPECT_THAT(s, StrEq(expected));

	EXPECT_THAT(ApiWrap::List_GetText(hWnd, 2, std::span{ std::data(s), std::size(expected) + 1 }), LB_ERR);
}

/*!
 * コンボボックスを対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, ComboTest001) {
	const auto expected = L"0123456789abcdef"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const auto hWnd = ::CreateWindowExW(0, WC_COMBOBOX, L"", 0, 1, 1, 1, 1, HWND(nullptr), HMENU(nullptr), hInstance, nullptr);

	WindowHolder windowHolder{ hWnd };

	// アイテムを追加
	ApiWrap::AddCbItems(hWnd, std::array{ expected, L""s });

	// アイテム数を取得
	EXPECT_THAT(ApiWrap::Combo_GetCount(hWnd), Eq(2));

	// 初期状態は未選択
	EXPECT_THAT(ApiWrap::Combo_GetCurSel(hWnd), Eq(CB_ERR));

	// プログラム的にアイテム選択する
	EXPECT_THAT(ApiWrap::Combo_SetCurSel(hWnd, 1), Eq(1));
	EXPECT_THAT(ApiWrap::Combo_GetCurSel(hWnd), Eq(1));

	// 指定したアイテムの表示文字列を取得（この機能はあまり重要でない）
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 0), StrEq(expected));
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 1), StrEq(L""));
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 2), IsFalse());

	// サイズ20の配列バッファを用意する
	StaticString<20> arrayBuffer{};

	// アイテムサイズに制限をかける
	ApiWrap::LimitCbText(hWnd, arrayBuffer);

	// 選択中アイテムのテキストを取得
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(L""));

	// 制限を無視してテキストを変更（変更できてしまう仕様）
	ApiWrap::SetWindowTextW(hWnd, std::format(L"{:a<20}", L'a'));
	EXPECT_THAT(ApiWrap::GetWindowTextW(hWnd), StrEq(std::format(L"{:a<20}", L'a')));

	// アイテムを追加（追加できてしまう仕様）
	ApiWrap::AddCbItems(hWnd, std::array{ std::format(L"{:a<20}", L'a') });
	EXPECT_THAT(ApiWrap::GetCbItemText(hWnd, 2), StrEq(std::format(L"{:a<20}", L'a')));

	CNativeW cmemText;
	EXPECT_THAT(ApiWrap::Combo_GetLBText(hWnd, 0, cmemText), int(std::size(expected)));
	EXPECT_THAT(cmemText.GetStringPtr(), StrEq(expected));
}

/*!
 * ダイアログボックス項目を対象とするラッパーメソッドのテスト
 */
TEST(ApiWrap, DlgItemTest001) {
	const auto expected = L"0123456789abcdef"s;

	const auto hInstance = ::GetModuleHandleW(nullptr);
	const HWND hWnd = nullptr;

	struct CDlgTest : public CDialog {
		CDlgTest() noexcept
			: CDialog(false, false)
		{
		}

		bool OnInitDialog(HWND hWndDlg, HWND hWndFocus, LPARAM lParam) override {
			const auto hDlg = hWndDlg;
			const auto bRet = CDialog::OnInitDialog(hWndDlg, hWndFocus, lParam);
			OnFirstIdle(hDlg);

			::EndDialog(hDlg, IDCANCEL);

			return TRUE;
		}

		void OnFirstIdle(HWND hDlg) const {
			// アイテムにテキストを設定しておく
			ApiWrap::SetDlgItemTextW(hDlg, IDC_COMBO_TEXT, L"test item"s);
			EXPECT_THAT(ApiWrap::GetDlgItemTextW(hDlg, IDC_COMBO_TEXT), StrEq(L"test item"));

			// 存在しないIDを指定して取得失敗させる
			EXPECT_THAT(ApiWrap::GetDlgItemTextW(hDlg, IDC_COMBO_TEXT2), IsFalse());

			// チェックボックスの状態を確認する
			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsFalse());

			// チェックボックスの状態を変更して確認する
			ApiWrap::CheckDlgButton(hDlg, IDC_CHK_REGULAREXP, true);
			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsTrue());

			// チェックボックスの状態を元に戻せることを確認する
			ApiWrap::CheckDlgButton(hDlg, IDC_CHK_REGULAREXP, false);
			EXPECT_THAT(ApiWrap::IsDlgButtonChecked(hDlg, IDC_CHK_REGULAREXP), IsFalse());

			// チェックボックスの表示状態を変更する
			ApiWrap::ShowDlgItem(hDlg, IDC_CHK_REGULAREXP, false);

			// チェックボックスの表示状態を元に戻す
			ApiWrap::ShowDlgItem(hDlg, IDC_CHK_REGULAREXP, true);
		}
	};

	CDlgTest cDlg{};
	cDlg.DoModal(hInstance, hWnd, IDD_FIND, 0L);
}

} // namespace window
