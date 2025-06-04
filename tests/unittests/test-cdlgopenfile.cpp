/*! @file */
/*
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
#include "pch.h"
#include "dlg/CDlgOpenFile.h"

#include "env/CShareData.h"

#include "testing/GuiAwareTestSuite.hpp"

namespace file_dialog {

struct CDlgOpenFileTest : public testing::TGuiAware<::testing::Test> {
	using Base = testing::TGuiAware<::testing::Test>;

	static inline std::unique_ptr<CShareData> gm_pShareData = nullptr;

	static inline HINSTANCE hInstance = GetModuleHandle(nullptr);
	static inline HWND hWnd = nullptr;

	static auto& GetShareData() { return GetDllShareData(); }

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// OLEを初期化する
		EXPECT_TRUE(Base::SetUpGuiTestSuite());

		if (gm_OleInitialized) {
			// 共有メモリのインスタンスを生成する
			gm_pShareData = std::make_unique<CShareData>();

			// 共有メモリのインスタンスを初期化する
			EXPECT_TRUE(gm_pShareData->InitShareData());
		}
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		gm_pShareData = nullptr;

		// OLEの初期化を解除する
		Base::TearDownGuiTestSuite();
	}

	CDlgOpenFile cDlgOpenFile;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// UI Automationオブジェクトを作成する
		Base::SetUp();

		GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// 設定を元に戻す
		GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = true;
	}
};

TEST_F(CDlgOpenFileTest, CommonItemDialogCreate)
{
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.txt",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_F(CDlgOpenFileTest, CommonFileDialogCreate)
{
	GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.txt",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_F(CDlgOpenFileTest, CommonItemDialogDefaltFilterLong)
{
	// 落ちたり例外にならないこと
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L".extension_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_F(CDlgOpenFileTest, CommonFileDialogDefaltFilterLong)
{
	GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	// 落ちたり例外にならないこと
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_250_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_F(CDlgOpenFileTest, CommonFileDialogDefaltFilterMany)
{
	GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	// 落ちたり例外にならないこと
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_50_0_long_long_long_long_long_long_LONG;*.extension_50_1_long_long_long_long_long_long_LONG;*.extension_50_2_long_long_long_long_long_long_LONG;*.extension_50_3_long_long_long_long_long_long_LONG;*.extension_50_4_long_long_long_long_long_long_LONG;*.extension_50_5_long_long_long_long_long_long_LONG;*.extension_50_6_long_long_long_long_long_long_LONG;*.extension_50_7_long_long_long_long_long_long_LONG;*.extension_50_8_long_long_long_long_long_long_LONG;*.extension_50_9_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_F(CDlgOpenFileTest, CommonItemDialogDefaltFilterMany)
{
	// 落ちたり例外にならないこと
	cDlgOpenFile.Create(
		GetModuleHandle(nullptr),
		nullptr,
		L"*.extension_50_0_long_long_long_long_long_long_LONG;*.extension_50_1_long_long_long_long_long_long_LONG;*.extension_50_2_long_long_long_long_long_long_LONG;*.extension_50_3_long_long_long_long_long_long_LONG;*.extension_50_4_long_long_long_long_long_long_LONG;*.extension_50_5_long_long_long_long_long_long_LONG;*.extension_50_6_long_long_long_long_long_long_LONG;*.extension_50_7_long_long_long_long_long_long_LONG;*.extension_50_8_long_long_long_long_long_long_LONG;*.extension_50_9_long_long_long_long_long_long_LONG",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);
}

TEST_F(CDlgOpenFileTest, DoModal_GetOpenFileName001)
{
	GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	cDlgOpenFile.Create(
		hInstance,
		hWnd,
		L"*.*",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);

	SFilePath szPath = LR"(C:\Windows\System32\drivers\etc\hosts)";

	std::thread t([&] {
		cDlgOpenFile.DoModal_GetOpenFileName(
			szPath,
			EFilter::EFITER_TEXT
		);
	});

	const auto hWndDlgGetOpenFileName = WaitForDialog(L"開く");

	WaitForClose(hWndDlgGetOpenFileName, [this, hWndDlgGetOpenFileName] () {
		EmulateInvokeButton(hWndDlgGetOpenFileName, L"キャンセル");
	});

	t.join();
}

TEST_F(CDlgOpenFileTest, DISABLED_DoModal_GetOpenFileName002)
{
	GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	cDlgOpenFile.Create(
		hInstance,
		hWnd,
		L"*.*",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);

	SFilePath szPath = LR"(C:\Windows\System32\drivers\etc\hosts)";

	std::thread t([&] {
		cDlgOpenFile.DoModal_GetOpenFileName(
			szPath,
			EFilter::EFITER_TEXT
		);
	});

	const auto& hWndDlgGetOpenFileName = WaitForDialog(L"開く");

	auto filePath = GetExeFileName();

	// ファイル名を入力
	const auto pFileName = Base::GetFocusedElement();
	EmulateSetValue(pFileName, filePath.wstring());

	WaitForClose(hWndDlgGetOpenFileName, [this, hWndDlgGetOpenFileName] () {
		EmulateInvokeButton(hWndDlgGetOpenFileName, L"開く(O)");
	});

	t.join();

	EXPECT_THAT(szPath, StrEq(filePath));
}

TEST_F(CDlgOpenFileTest, DoModal_GetSaveFileName001)
{
	GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	cDlgOpenFile.Create(
		hInstance,
		hWnd,
		L"*.*",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);

	SFilePath szPath = LR"(C:\Windows\System32\drivers\etc\hosts)";

	std::thread t([&] {
		cDlgOpenFile.DoModal_GetSaveFileName(szPath);
	});

	const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存");

	WaitForClose(hWndDlgSaveFileName, [this, hWndDlgSaveFileName] () {
		EmulateInvokeButton(hWndDlgSaveFileName, L"キャンセル");
	});

	t.join();
}

TEST_F(CDlgOpenFileTest, DISABLED_DoModal_GetSaveFileName002)
{
	GetShareData().m_Common.m_sEdit.m_bVistaStyleFileDialog = false;
	cDlgOpenFile.Create(
		hInstance,
		hWnd,
		L"*.*",
		L"C:\\Windows",
		std::vector<LPCWSTR>(),
		std::vector<LPCWSTR>()
	);

	SFilePath szPath = LR"(C:\Windows\System32\drivers\etc\hosts)";

	std::thread t([&] {
		cDlgOpenFile.DoModal_GetSaveFileName(szPath);
	});

	const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存");

	auto filePath = GetExeFileName();

	// ファイル名を入力
	const auto pFileName = Base::GetFocusedElement();
	EmulateSetValue(pFileName, filePath.wstring());

	WaitForClose(hWndDlgSaveFileName, [this, hWndDlgSaveFileName] () {
		EmulateInvokeButton(hWndDlgSaveFileName, L"保存(S)");
	});

	t.join();

	EXPECT_THAT(szPath, StrEq(filePath));
}

//bool DoModal_GetSaveFileName( WCHAR* pszPath ) override;
//bool DoModalOpenDlg( SLoadInfo* pLoadInfo,
//	std::vector<std::wstring>* pFilenames,
//	bool bOptions = true ) override;
//bool DoModalSaveDlg( SSaveInfo* pSaveInfo, bool bSimpleMode ) override;

} // namespace file_dialog
