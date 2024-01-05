﻿/*! @file */
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
#include "dlg/CDialog.h"
#include "dlg/CSizeRestorableDialog.hpp"

#include "MockShareDataAccessor.hpp"

#include "MockUser32Dll.hpp"

#include "TAutoCloseDialog.hpp"

extern HINSTANCE GetLanguageResourceLibrary();

/*
 * ダイアログクラステンプレートをテストするためのクラス
 
 * 自動テストで実行できるように作成したもの。
 * 初期表示後、勝手に閉じる仕様。
 */
class CDialog1 : public TAutoCloseDialog<CDialog, IDC_EDIT_INPUT1>
{
	DLGPROC _pfnDlgProc = nullptr;

public:
	explicit CDialog1(const User32Dll& User32Dll_ = ::GetUser32Dll());
	~CDialog1() override = default;

	using CDialog::DispatchDlgEvent;

protected:
	BOOL    OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam) override;
};

/*!
 * コンストラクター
 */
CDialog1::CDialog1(const User32Dll& User32Dll_)
	: TAutoCloseDialog(IDD_INPUT1, User32Dll_)
{
}

/*!
 * WM_INITDIALOG処理
 *
 * ダイアログ構築後、最初に受け取るメッセージを処理する。
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] hWndFocus フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE  フォーカスが設定されます。
 * @retval FALSE フォーカスは設定されません。
 */
BOOL CDialog1::OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
	// 派生元クラスに処理を委譲する
	const auto ret = __super::OnDlgInitDialog(hDlg, hWndFocus, lParam);

	// デフォルト実装は1を返す
	assert(1 == GetDlgData(hDlg));

	// 派生元クラスが返した戻り値をそのまま返す
	return ret;
}

class mock_dialog_1 : public CDialog1
{
public:
	explicit mock_dialog_1(const User32Dll& User32Dll_ = ::GetUser32Dll())
		: CDialog1(User32Dll_)
	{
	}

	MOCK_METHOD3_T(OnInitDialog, BOOL(HWND, WPARAM, LPARAM));
	MOCK_METHOD0_T(OnDestroy, BOOL());
	MOCK_METHOD2_T(OnMove, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnCommand, BOOL(WPARAM, LPARAM));
	MOCK_METHOD1_T(OnNotify, BOOL(LPNMHDR));
	MOCK_METHOD1_T(OnTimer, BOOL(WPARAM));
	MOCK_METHOD2_T(OnKeyDown, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnKillFocus, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnActivate, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnPopupHelp, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnContextMenu, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2_T(OnDrawItem, BOOL(WPARAM, LPARAM));
};

/*
 * ダイアログクラステンプレートをテストするためのクラス

 * 自動テストで実行できるように作成したもの。
 */
class CDialog2 : public CSizeRestorableDialog
{
public:
	static constexpr auto DIALOG_ID = IDD_INPUT1;

	explicit CDialog2(const ShareDataAccessor& ShareDataAccessor_, const User32Dll& User32Dll_ = ::GetUser32Dll());
	~CDialog2() override = default;

	using CDialog::DispatchDlgEvent;
};

/*!
 * コンストラクター
 */
CDialog2::CDialog2(const ShareDataAccessor& ShareDataAccessor_, const User32Dll& User32Dll_)
	: CSizeRestorableDialog(DIALOG_ID, ShareDataAccessor_, User32Dll_)
{
}

class mock_dialog_2 : public CDialog2
{
public:
	explicit mock_dialog_2(const ShareDataAccessor& ShareDataAccessor_, const User32Dll& User32Dll_ = ::GetUser32Dll())
		: CDialog2(ShareDataAccessor_, User32Dll_)
	{
	}

	MOCK_METHOD2_T(OnSize, BOOL(WPARAM, LPARAM));
	MOCK_METHOD2(OnGetMinMaxInfo, void(HWND hDlg, _In_ LPMINMAXINFO lpMinMaxInfo));
};

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

/*!
 * モーダルダイアログ表示、正常系テスト
 */
TEST(CDialog, SimpleDoModal)
{
	const auto hInstance  = static_cast<HINSTANCE>(nullptr);
	const auto hWndParent = static_cast<HWND>(nullptr);
	const auto lParam     = static_cast<LPARAM>(0);
	CDialog1   dlg;
	EXPECT_EQ(IDOK, dlg.DoModal(hInstance, hWndParent, IDD_INPUT1, lParam));
}

/*!
 * モーダルダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CDialog, MockedDoModal)
{
	// メッセージリソースDLLのインスタンスハンドル
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	const auto hInstance  = static_cast<HINSTANCE>(nullptr);
	const auto hWndParent = HWND(0x1234);
	const auto lParam     = static_cast<LPARAM>(0);

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, DialogBoxParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_INPUT1), hWndParent, _, _)).WillOnce(Return(IDCANCEL));

	mock_dialog_1 mock(*pUser32Dll);
	EXPECT_EQ(IDCANCEL, mock.DoModal(hInstance, hWndParent, IDD_INPUT1, lParam));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CSizeRestorableDialog, SimpleDoModeless1)
{
	const auto hInstance  = static_cast<HINSTANCE>(nullptr);
	const auto hWndParent = static_cast<HWND>(nullptr);
	const auto lParam     = static_cast<LPARAM>(0);
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CDialog2 dlg(*pShareDataAccessor);
	const auto hDlg = dlg.DoModeless(hInstance, hWndParent, IDD_COMPARE, lParam, SW_SHOW);
	EXPECT_TRUE(hDlg);
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CSizeRestorableDialog, MockedDoModeless1)
{
	// メッセージリソースDLLのインスタンスハンドル
	const auto hLangRsrcInstance = GetLanguageResourceLibrary();

	const auto hInstance  = static_cast<HINSTANCE>(nullptr);
	const auto hWndParent = HWND(0x1234);
	const auto lParam     = static_cast<LPARAM>(0);

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, CreateDialogParamW(hLangRsrcInstance, MAKEINTRESOURCEW(IDD_COMPARE), hWndParent, _, _)).WillOnce(Return(hDlg));
	EXPECT_CALL(*pUser32Dll, ShowWindow(hDlg, SW_SHOW)).WillOnce(Return(true));

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 dlg(*pShareDataAccessor, *pUser32Dll);
	EXPECT_EQ(hDlg, dlg.DoModeless(hInstance, hWndParent, IDD_COMPARE, lParam, SW_SHOW));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 */
TEST(CSizeRestorableDialog, SimpleDoModeless2)
{
	const auto hWndParent = static_cast<HWND>(nullptr);
	const auto lParam     = static_cast<LPARAM>(0);

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CDialog2 dlg(*pShareDataAccessor);
	const auto hDlg = dlg.DoModeless2(hWndParent, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, lParam, SW_SHOWDEFAULT);
	EXPECT_TRUE(hDlg);
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CSizeRestorableDialog, MockedDoModeless2_fail)
{
	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;
	const auto lParam     = static_cast<LPARAM>(0);

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, FindResourceW(_, _, _)).WillOnce(Return(nullptr));
	EXPECT_CALL(*pUser32Dll, LoadResource(_, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, LockResource(_)).Times(0);
	EXPECT_CALL(*pUser32Dll, SizeofResource(_, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, CreateDialogIndirectParamW(_, _, _, _, _)).Times(0);
	EXPECT_CALL(*pUser32Dll, ShowWindow(_, _)).Times(0);

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 dlg(*pShareDataAccessor, *pUser32Dll);
	EXPECT_EQ(nullptr, dlg.DoModeless2(hWndParent, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, lParam, SW_SHOWDEFAULT));
}

/*!
 * モードレスダイアログ表示、正常系テスト
 *
 * Windows APIの呼び出しパラメーターを確認する
 */
TEST(CSizeRestorableDialog, MockedDoModeless2)
{
	// 親ウインドウのハンドル(ダミー)
	const auto hWndParent = (HWND)0x1234;
	const auto lParam     = static_cast<LPARAM>(0);

	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, FindResourceW(_, _, _)).WillOnce(Invoke(::FindResourceW));
	EXPECT_CALL(*pUser32Dll, LoadResource(_, _)).WillOnce(Invoke(::LoadResource));
	EXPECT_CALL(*pUser32Dll, LockResource(_)).WillOnce(Invoke(::LockResource));
	EXPECT_CALL(*pUser32Dll, SizeofResource(_, _)).WillOnce(Invoke(::SizeofResource));
	EXPECT_CALL(*pUser32Dll, CreateDialogIndirectParamW(_, _, hWndParent, _, _)).WillOnce(Return(hDlg));
	EXPECT_CALL(*pUser32Dll, ShowWindow(hDlg, SW_SHOWDEFAULT)).WillOnce(Return(TRUE));

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 dlg(*pShareDataAccessor, *pUser32Dll);
	EXPECT_EQ(hDlg, dlg.DoModeless2(hWndParent, [](DLGTEMPLATE& dlgTemplate) { dlgTemplate.style = WS_OVERLAPPEDWINDOW | DS_SETFONT; }, lParam, SW_SHOWDEFAULT));
}

TEST(CDialog, MockedDispachDlgEvent_OnInitDialog)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	const auto hWndFocus = (HWND)0x1234;

	mock_dialog_1 mock;

	auto wParam = (WPARAM)hWndFocus;
	auto lParam = LPARAM(&mock);

	EXPECT_CALL(mock, OnInitDialog(hDlg, wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_INITDIALOG, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnMove)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnMove(_, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_MOVE, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnCommand)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnCommand(wParam, lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_COMMAND, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnNotify)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnNotify((NMHDR*)lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_NOTIFY, 0, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnTimer)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnTimer(wParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_TIMER, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnKeyDown)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnKeyDown(wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_KEYDOWN, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnActivate)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnActivate(wParam, lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_ACTIVATE, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnKillFocus)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnKillFocus(wParam, _)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_KILLFOCUS, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnPopupHelp)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnPopupHelp(_, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_HELP, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnContextMenu)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnContextMenu(wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_CONTEXTMENU, wParam, lParam));
}

TEST(CDialog, MockedDispachDlgEvent_OnSize)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 mock(*pShareDataAccessor);
	EXPECT_CALL(mock, OnSize(wParam, lParam)).WillOnce(Return(false));

	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_SIZE, wParam, lParam));
}

TEST(CSizeRestorableDialog, MockedDispachDlgEvent_OnGetMinMaxInfo)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	// サイズ情報
	MINMAXINFO minMaxInfo = {};

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	mock_dialog_2 mock(*pShareDataAccessor);
	EXPECT_CALL(mock, OnGetMinMaxInfo(hDlg, &minMaxInfo)).Times(1);
	EXPECT_FALSE(mock.DispatchDlgEvent(hDlg, WM_GETMINMAXINFO, 0, std::bit_cast<LPARAM>(&minMaxInfo)));

	CDialog2 dlg(*pShareDataAccessor);
	EXPECT_FALSE(dlg.DispatchDlgEvent(hDlg, WM_GETMINMAXINFO, 0, std::bit_cast<LPARAM>(&minMaxInfo)));
}

TEST(CSizeRestorableDialog, MockedDispachDlgEvent_OnDrawItem)
{
	// 作成されたウインドウのハンドル(ダミー)
	const auto hDlg = (HWND)0x4321;

	auto wParam = (WPARAM)0x1111;
	auto lParam = (LPARAM)0x2222;

	mock_dialog_1 mock;
	EXPECT_CALL(mock, OnDrawItem(wParam, lParam)).WillOnce(Return(true));

	EXPECT_TRUE(mock.DispatchDlgEvent(hDlg, WM_DRAWITEM, wParam, lParam));
}

TEST(CDialog, GetHelpIdTable)
{
	mock_dialog_1 dlg;
	EXPECT_TRUE(dlg.GetHelpIdTable());
}
