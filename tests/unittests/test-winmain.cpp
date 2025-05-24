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
#include "testing/StartEditorProcess.hpp"

#include "testing/GuiAwareTestSuite.hpp"

#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "_main/CControlTray.h"
#include "config/system_constants.h"
#include "version.h"

#include "io/CZipFile.h"
#include "CDataProfile.h"

#include "tests1_rc.h"

#define RT_ZIPRES MAKEINTRESOURCE(101)

#define ID_HOTKEY_TRAYMENU	0x1234

#if defined(_MSC_VER)
#  define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#  define NORETURN __attribute__((noreturn))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define NORETURN _Noreturn
#else
#  define NORETURN
#endif

using BinarySequence = std::basic_string<std::byte>;
using BinarySequenceView = std::basic_string_view<std::byte>;

BinarySequence CopyBinaryFromResource(uint16_t nResourceId, LPCWSTR resource_type);
bool WriteBinaryToFile(BinarySequenceView bin, std::filesystem::path path);
std::filesystem::path GetTempFilePath(std::wstring_view prefix, std::wstring_view extension);

//! HANDLE型のスマートポインタ
using HandleHolder = cxx_util::ResourceHolder<HANDLE, &CloseHandle>;

//! コンテキストメニューのスマートポインタ
using ContextMenuHolder = cxx_util::ResourceHolder<HWND, &DestroyWindow>;

namespace testing {

/*!
 * GUIテストのための共通クラス
 *
 */
template<typename BaseTestSuiteType>
class TSakuraGuiAware : public TGuiAware<BaseTestSuiteType> {
protected:
	using Base = TGuiAware<BaseTestSuiteType>;

public:
	/*!
	 * プロファイル名
	 */
	static inline std::wstring_view gm_ProfileName = L""sv;

	/*!
	 * 設定ファイルのパス
	 *
	 * CShareData::BuildPrivateIniFileNameを使ってtests1.iniのパスを取得する。
	 */
	static inline std::filesystem::path gm_IniPath;

	/*!
	 * テスト用1000行データファイルのパス
	 */
	static inline std::filesystem::path gm_TestFilePath1 = std::filesystem::current_path() / L"test_1000lines.txt";

	/*!
	 * テスト用2000行データファイルのパス
	 */
	static inline std::filesystem::path gm_TestFilePath2 = std::filesystem::current_path() / L"test_2000lines.txt";

	/*!
	 * テスト用エクスポートされたタイプ設定ファイルのパス
	 */
	static inline std::filesystem::path gm_ExportedTypeFilePath = std::filesystem::current_path() / L"テキスト.ini";

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpFuncTest() {
		SFilePath szCommand;
		if (gm_ProfileName.length()) {
			szCommand = strprintf(LR"(-PROF="%s")", gm_ProfileName.data());
		}

		// コマンドラインのインスタンスを用意する
		CCommandLine commandLine;
		commandLine.ParseCommandLine(szCommand, false);

		// プロセスのインスタンスを用意する
		CControlProcess dummy(nullptr, szCommand);

		const auto isMultiUserSettings = false;
		const auto userRootFolder = 0;
		const auto& userSubFolder = L"sakura";

		// exe基準のiniファイルパスを得る
		const auto defaultIniPath = GetExeFileName().replace_extension(L".ini");

		// 設定ファイルフォルダー
		auto iniFolder = defaultIniPath;
		iniFolder.remove_filename();

		// iniファイル名を得る
		const auto filename = defaultIniPath.filename();

		// INIファイルのパスを組み立てる
		gm_IniPath = CShareData::BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, gm_ProfileName.data(), filename);

		// INIファイルを削除する
		if (fexist(gm_IniPath)) {
			std::filesystem::remove(gm_IniPath);
		}

		// テスト用ファイル作成
		std::wofstream fs(gm_TestFilePath1);
		for (int n = 1; n <= 1000; n++) {
			fs << n << std::endl;
		}
		fs.close();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownFuncTest() {
		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		// テスト用ファイルを削除
		if (fexist(gm_ExportedTypeFilePath)) {
			std::filesystem::remove(gm_ExportedTypeFilePath);
		}

		if (fexist(gm_TestFilePath2)) {
			std::filesystem::remove(gm_TestFilePath2);
		}

		if (fexist(gm_TestFilePath1)) {
			std::filesystem::remove(gm_TestFilePath1);
		}

		// INIファイルを削除する
		if (fexist(gm_IniPath)) {
			std::filesystem::remove(gm_IniPath);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (gm_ProfileName.length()) {
			std::filesystem::remove(gm_IniPath.parent_path());
		}
	}

	/*!
	 * @brief キャプション文字列を構築します。
	 */
	static std::wstring BuildCaption(std::wstring_view title)
	{
		const auto szAppName = SFilePath(GSTR_APPNAME);
		const auto versionMS = MAKELONG(VER_B, VER_A);
		const auto versionLS = MAKELONG(VER_D, VER_C);

		return strprintf(L"%s - %s %d.%d.%d.%d  ",
			title.data(),
			szAppName.c_str(),
			HIWORD(versionMS),
			LOWORD(versionMS),
			HIWORD(versionLS),
			LOWORD(versionLS)
		);
	}

	static void StartSakuraProcess(const std::vector<std::wstring>& args) {
		CProcess::StartSakuraProcess(gm_ProfileName.data(), args);
	}

	static void StartControlProcess();
	static void WaitForControlProcess();

	static HWND FindTrayWindow() {
		return CControlTray::Find(gm_ProfileName.data());
	}

	static void TerminateControlProcess();

	/*!
	 * コンストラクタは流用
	 */
	using Base::Base;

protected:
	_Success_(return != nullptr)
	HWND WaitForTrayWindow() const
	{
		SFilePath szEditAppName = GSTR_CEDITAPP;
		if (gm_ProfileName.length()) {
			szEditAppName += gm_ProfileName;
		}

		HWND hWndFound = nullptr;

		// ウィンドウがVisibleかつEnabledになるのを待つ
		const auto startTick = GetTickCount64();
		while (!hWndFound && GetTickCount64() - startTick < Base::defaultTimeoutMillis) {
			hWndFound = FindWindowW(szEditAppName, szEditAppName);
			if (!hWndFound) {
				//do nothing
			}
			else if (!IsWindowEnabled(hWndFound)) {
				hWndFound = nullptr;
			}
	
			Sleep(10);  // 10msスリープしてリトライ
		}

		EXPECT_THAT(hWndFound, Ne(nullptr));

		if (hWndFound) {
			auto pWnd = Base::ElementFromHandle(hWndFound);

			EXPECT_THAT(pWnd, Ne(nullptr));
		}

		return hWndFound;
	}

	_Success_(return != nullptr)
	HWND WaitForEditor(
		std::optional<std::wstring_view> caption = std::nullopt
	) const
	{
		const auto windowName = caption.has_value() ? caption.value().data() : nullptr;

		const auto hWndEditor = Base::WaitForWindow(GSTR_EDITWINDOWNAME, windowName);

		EXPECT_THAT(hWndEditor, Ne(nullptr));

		return hWndEditor;
	}

	void SendEditorCommand(
		_In_ HWND hWndEditor,
		EFunctionCode eFuncCd
	) const
	{
		auto pEditor = Base::ElementFromHandle(hWndEditor);
		_com_util::CheckError(pEditor->SetFocus());

		SendMessageTimeoutW(hWndEditor, WM_COMMAND, MAKEWPARAM(eFuncCd, 0), 0,
			SMTO_BLOCK
			| SMTO_ABORTIFHUNG
			| SMTO_ERRORONEXIT
			,
			Base::defaultTimeoutMillis / 4,
			nullptr
		);
	}

	void EmulateSetOpenFileName(
		_In_ HWND hWndDlgSaveFileName,
		const std::filesystem::path& path
	) const
	{
		auto pFileName = Base::GetFocusedElement();
		EXPECT_THAT(pFileName, Ne(nullptr));
		this->EmulateSetValue(pFileName, path.wstring());

		this->WaitForClose(hWndDlgSaveFileName, [this, hWndDlgSaveFileName] () {
			auto pNameCondition = this->CreatePropertyCondition(UIA_NamePropertyId, L"開く(O)");
			auto pOpenFile = Base::FindFirst(hWndDlgSaveFileName, TreeScope_Subtree, pNameCondition);
			EXPECT_THAT(pOpenFile, Ne(nullptr));
			this->EmulateInvoke(pOpenFile);
		});
	}
};

/*!
 * @brief コントロールプロセスの初期化完了を待つ
 *
 * CEditorProcess::WaitForControlProcessとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::WaitForInitializedControlProcess()と概ね等価です。
 */
template<typename BaseTestSuiteType>
/* static */ void TSakuraGuiAware<BaseTestSuiteType>::WaitForControlProcess()
{
	// 初期化完了イベントを作成する
	SFilePath szEventName = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	if (gm_ProfileName.length()) {
		szEventName += gm_ProfileName;
	}

	HandleHolder hEvent = CreateEventW(nullptr, TRUE, FALSE, szEventName);
	if (!hEvent) {
		throw basis::message_error(L"create event failed.");
	}

	// 初期化完了イベントを待つ
	if (const auto dwRet = WaitForSingleObject(hEvent, 30000); WAIT_TIMEOUT == dwRet) {
		throw basis::message_error(L"waitEvent is timeout.");
	}
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * CControlProcess::Startとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存CProcessFactory::StartControlProcess()と概ね等価です。
 */
template<typename BaseTestSuiteType>
/* static */ void TSakuraGuiAware<BaseTestSuiteType>::StartControlProcess()
{
	// コントロールプロセスを起動する
	StartSakuraProcess({ L"-NOWIN" });

	// コントロールプロセスの初期化完了を待つ
	WaitForControlProcess();
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 *
 * CControlProcess::Terminateとして実装したいコードです。本体を変えたくないので一時定義しました。
 * 既存コードに該当する処理はありません。
 */
template<typename BaseTestSuiteType>
/* static */ void TSakuraGuiAware<BaseTestSuiteType>::TerminateControlProcess()
{
	// トレイウインドウを検索する
	const auto hTrayWnd = FindTrayWindow();
	if (!hTrayWnd) {
		// ウインドウがなければそのまま抜ける
		return;
	}

	// トレイウインドウからプロセスIDを取得する
	DWORD dwControlProcessId = 0;
	if (!GetWindowThreadProcessId(hTrayWnd, &dwControlProcessId) || !dwControlProcessId) {
		throw basis::message_error(L"dwControlProcessId can't be retrived.");
	}

	// プロセス情報の問い合せを行うためのハンドルを開く
	HandleHolder hControlProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwControlProcessId);
	if (!hControlProcess) {
		throw basis::message_error(L"hControlProcess can't be opened.");
	}

	// トレイウインドウを閉じる
	SendMessageW(hTrayWnd, WM_CLOSE, 0, 0);

	// プロセス終了を待つ
	if (DWORD dwExitCode = 0; GetExitCodeProcess(hControlProcess, &dwExitCode) && dwExitCode == STILL_ACTIVE) {
		if (const auto waitProcessResult = WaitForSingleObject(hControlProcess, INFINITE); WAIT_TIMEOUT == waitProcessResult) {
			throw basis::message_error(L"waitProcess is timeout.");
		}
	}
}

} // namespace testing

namespace func {

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class WinMainTest : public testing::TSakuraGuiAware<::testing::TestWithParam<std::wstring_view>> {
protected:
	using Base = testing::TSakuraGuiAware<::testing::TestWithParam<std::wstring_view>>;

public:
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// OLEを初期化する
		EXPECT_TRUE(SetUpGuiTestSuite());
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// OLEの初期化を解除する
		TearDownGuiTestSuite();
	}

	/*!
	 * コンストラクタは流用
	 */
	using Base::Base;

protected:
	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// プロファイル名を取得する
		Base::gm_ProfileName = GetParam();

		// UI Automationオブジェクトを作成する
		Base::SetUp();

		// 機能テストを初期化する
		SetUpFuncTest();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		// 機能テストをクリーンアップする
		TearDownFuncTest();

		// UI Automationオブジェクトを解放する
		Base::TearDown();
	}

	/*!
	 * @brief コントロールプロセスを起動し、終了指示を出して、終了を待つ
	 */
	static void StartAndTerminateControlProcess()
	{
		// コントロールプロセスを起動する
		StartControlProcess();

		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		// コントロールプロセスが終了すると、INIファイルが作成される
		auto iniCreated = fexist(gm_IniPath);
		const auto startTick = GetTickCount64();
		while (iniCreated && GetTickCount64() - startTick < defaultTimeoutMillis) {
			Sleep(10);  // 10msスリープしてリトライ
			iniCreated = fexist(gm_IniPath);
		}
	}

	/*!
	 * wWinMain呼出ラッパー
	 *
	 * EXPECT_EXITやEXPECT_DEATHで使うことを意図しています。
	 */
	static NORETURN void StartEditorProcess(const std::wstring& command) {
		exit(testing::StartEditorProcess(command));
	}
};

/*!
 * @brief wWinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コントロールプロセスを実行する。
 *  プロセス起動は2回行い、1回目でINI作成＆書き込み、2回目でINI読み取りを検証する。
 */
TEST_P(WinMainTest, runWithNoWin)
{
	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	StartAndTerminateControlProcess();

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_TRUE(fexist(gm_IniPath));

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	StartAndTerminateControlProcess();

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_TRUE(fexist(gm_IniPath));
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST_P(WinMainTest, runEditorProcess)
{
	// 起動時実行マクロの中身を作る
	constexpr std::array macroCommands = {
		L"Down();"sv,
		L"Up();"sv,
		L"Right();"sv,
		L"Left();"sv,

		L"Outline(0);"sv,				// アウトライン解析

		L"ShowFunckey();"sv,			// ShowFunckey 出す
		L"ShowMiniMap();"sv,			// ShowMiniMap 出す
		L"ShowTab();"sv,				// ShowTab 出す

		L"SelectAll();"sv,
		L"GoFileEnd();"sv,
		L"GoFileTop();"sv,

		L"PrintPreview();"sv,
		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,
		L"PrintPreview();"sv,

		L"SplitWinVH();"sv,
		L"NextWindow();NextWindow();NextWindow();NextWindow();"sv,

		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,

		L"PrevWindow();PrevWindow();PrevWindow();PrevWindow();"sv,
		L"SplitWinVH();"sv,

		L"ShowFunckey();"sv,			//ShowFunckey 消す
		L"ShowMiniMap();"sv,			//ShowMiniMap 消す
		L"ShowTab();"sv,				//ShowTab 消す

		L"ExpandParameter('$I');"sv,	// INIファイルパスの取得(呼ぶだけ)

		// フォントサイズ設定のテスト(ここから)
		L"SetFontSize(0, 1, 0);"sv,		// 相対指定 - 拡大 - 対象：共通設定
		L"SetFontSize(0, -1, 0);"sv,	// 相対指定 - 縮小 - 対象：共通設定
		L"SetFontSize(100, 0, 0);"sv,	// 直接指定 - 対象：共通設定
		L"SetFontSize(100, 0, 1);"sv,	// 直接指定 - 対象：タイプ別設定
		L"SetFontSize(100, 0, 2);"sv,	// 直接指定 - 対象：一時適用
		L"SetFontSize(100, 0, 3);"sv,	// 直接指定 - 対象が不正
		L"SetFontSize(0, 0, 0);"sv,		// 直接指定 - フォントサイズ下限未満
		L"SetFontSize(9999, 0, 0);"sv,	// 直接指定 - フォントサイズ上限超過
		L"SetFontSize(0, 0, 2);"sv,		// 相対指定 - サイズ変化なし
		L"SetFontSize(0, 1, 2);"sv,		// 相対指定 - 拡大
		L"SetFontSize(0, -1, 2);"sv,	// 相対指定 - 縮小
		L"SetFontSize(0, 9999, 2);"sv,	// 相対指定 - 限界まで拡大
		L"SetFontSize(0, 1, 2);"sv,		// 相対指定 - これ以上拡大できない
		L"SetFontSize(0, -9999, 2);"sv,	// 相対指定 - 限界まで縮小
		L"SetFontSize(0, -1, 2);"sv,	// 相対指定 - これ以上縮小できない
		L"SetFontSize(100, 0, 2);"sv,	// 元に戻す
		// フォントサイズ設定のテスト(ここまで)

		L"Outline(2);"sv,	//アウトライン解析を閉じる

		L"ExitAll();"sv		//NOTE: このコマンドにより、エディタプロセスは起動された直後に終了する。
	};

	// 起動時実行マクロを組み立てる
	const auto strStartupMacro = std::accumulate(macroCommands.cbegin(), macroCommands.cend(), std::wstring(), [](const std::wstring& a, std::wstring_view b) { return a + std::data(b); });

	// コマンドラインを組み立てる
	std::wstring command(gm_TestFilePath1.wstring());
	command += strprintf(LR"( -PROF="%s")", gm_ProfileName.data());
	command += strprintf(LR"( -MTYPE=js -M="%s")", std::regex_replace(strStartupMacro, std::wregex(LR"(")"), LR"("")").c_str());

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	EXPECT_EXIT({ StartEditorProcess(command); }, ::testing::ExitedWithCode(0), ".*" );
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  プロファイルマネージャを表示してプロセスを起動する。
 */
TEST_P(WinMainTest, DISABLED_showProfileMgr001)
{
	// FIXME: プロファイル指定がある場合、起動されるプロセスとプロファイル名が合わない
	if (gm_ProfileName.length()) {
		return;
	}

	// コマンドラインを組み立てる
	const auto command = strprintf(LR"(-PROFMGR -PROF="%s")", gm_ProfileName.data());

	// wWinMainを別スレッドで実行する
	std::thread t([command] {
		testing::StartEditorProcess(command);
	});

	// プロファイルマネージャの表示を待つ
	if (const auto hWndDlgProfileMgr = WaitForDialog(L"プロファイルマネージャ")) {
		// ボタンを押下する
		EmulateInvokeButton(hWndDlgProfileMgr, L"起動(S)");
	}

	// 編集ウインドウが起動するのを待つ
	const auto hWndEditor = WaitForEditor();

	// 編集ウインドウを閉じる
	WaitForClose(hWndEditor, [this, hWndEditor] () {
		SendEditorCommand(hWndEditor, F_WINCLOSE);
	});

	t.join();
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  プロファイルマネージャを表示して何もせず閉じる。
 */
TEST_P(WinMainTest, DISABLED_showProfileMgr101)
{
	// コマンドラインを組み立てる
	const auto command = strprintf(LR"(-PROFMGR -PROF="%s")", gm_ProfileName.data());

	// wWinMainを別スレッドで実行する
	std::thread t([command] {
		testing::StartEditorProcess(command);
	});

	// プロファイルマネージャの表示を待つ
	if (const auto hWndDlgProfileMgr = WaitForDialog(L"プロファイルマネージャ")) {
		// プロファイルマネージャを閉じる
		WaitForClose(hWndDlgProfileMgr, [this, hWndDlgProfileMgr] () {
			EmulateInvokeButton(hWndDlgProfileMgr, L"閉じる(X)");
		});
	}

	t.join();

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_FALSE(fexist(gm_IniPath));
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  プロファイル指定なしとプロファイル指定ありの2パターンで実体化させる
 */
INSTANTIATE_TEST_SUITE_P(WinMain
	, WinMainTest
	, ::testing::Values(
		L"",
		L"profile1"
	)
);

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class TrayIconTest : public testing::TSakuraGuiAware<::testing::Test> {
protected:
	using Base = testing::TSakuraGuiAware<::testing::Test>;

public:
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	 static void SetUpTestSuite() {
		// OLEを初期化する
		EXPECT_TRUE(SetUpGuiTestSuite());

		// OLEの初期化に成功した場合
		if (gm_OleInitialized) {
			// 機能テストを初期化する
			SetUpFuncTest();

			// コントロールプロセスを起動する
			StartControlProcess();

			// コントロールプロセスに終了指示を出して終了を待つ
			TerminateControlProcess();

			// コントロールプロセスが終了すると、INIファイルが作成される
			EXPECT_TRUE(fexist(gm_IniPath));

			// 常駐設定にする
			CDataProfile cProfile;
			cProfile.ReadProfile(gm_IniPath.c_str());
			cProfile.SetProfileData(L"Common", L"bTaskTrayStay", true);
			cProfile.WriteProfile(gm_IniPath.c_str(), L"sakura.ini テキストエディタ設定ファイル");

			// コントロールプロセスを起動する
			StartControlProcess();
		}
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		// 機能テストをクリーンアップする
		TearDownFuncTest();

		// OLEの初期化を解除する
		TearDownGuiTestSuite();
	}

	static int GetContextMenuIndex(UINT uMsg, EFunctionCode eFuncCd)
	{
		ptrdiff_t index = -1;

		// 左クリック
		static constexpr std::array contextMenuItemsL = {
			F_FILENEW,
			F_FILEOPEN,
			F_GREP_DIALOG,
			F_FILE_USED_RECENTLY,
			F_FOLDER_USED_RECENTLY,
			F_FAVORITE,
			F_FILESAVEALL,
			F_EXITALLEDITORS,
			F_EXITALL
		};

		// 右クリック
		static constexpr std::array contextMenuItemsR = {
			F_HELP_CONTENTS,
			F_HELP_SEARCH,
			F_TYPE_LIST,
			F_OPTION,
			F_ABOUT,
			F_EXITALL
		};

		if (WM_RBUTTONUP == uMsg) {
			// 右クリック
			if (const auto it = std::find(std::begin(contextMenuItemsR), std::end(contextMenuItemsR), eFuncCd); it != std::end(contextMenuItemsR)) {
				index = std::distance(std::begin(contextMenuItemsR), it);
			}
		}
		else if (WM_LBUTTONUP == uMsg) {
			// 左クリック
			if (const auto it = std::find(std::begin(contextMenuItemsL), std::end(contextMenuItemsL), eFuncCd); it != std::end(contextMenuItemsL)) {
				index = std::distance(std::begin(contextMenuItemsL), it);
			}
		}

		if (index < 0 || std::numeric_limits<int>::max() < index) {
			throw basis::message_error(L"Invalid message type.");
		}

		return int(index);
	}

	/*!
	 * トレイウインドウのハンドル
	 */
	HWND hWndTray = nullptr;

	/*
	 * コンストラクタは流用
	 */
	using Base::Base;

private:
	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// UI Automationオブジェクトを作成する
		Base::SetUp();

		hWndTray = FindTrayWindow();

		if (!hWndTray) {
			// コントロールプロセスを起動する
			StartControlProcess();

			//トレイウィンドウを待機する
			hWndTray = FindTrayWindow();
		}

		EXPECT_THAT(hWndTray, Ne(nullptr));
	}

protected:
	void ExecTrayCommand(UINT uMsg, EFunctionCode eFuncCd) const
	{
		// 機能コードをメニューインデックスに変換する
		const auto index = GetContextMenuIndex(uMsg, eFuncCd);

		std::thread t([this, index] {
			// コンテキストメニューを待つ
			const auto hWndContextMenu = Base::WaitForContextMenu();

			EXPECT_THAT(hWndContextMenu, Ne(nullptr));

			// メニュー項目を選択する
			EmulateInvokeMenuItem(hWndContextMenu, index);
		});

		// コンテキストメニューを開く
		SendMessageTimeoutW(hWndTray, MYWM_NOTIFYICON, 0, uMsg,
			SMTO_BLOCK
			| SMTO_ABORTIFHUNG
			| SMTO_ERRORONEXIT
			,
			Base::defaultTimeoutMillis / 4,
			nullptr
		);

		t.join();
	}

	auto WaitForTrayDialog(
		UINT uMsg,
		EFunctionCode eFuncCd,
		std::wstring_view title
	) const
	{
		// コンテキストメニューを待つ
		ExecTrayCommand(uMsg, eFuncCd);
		
		return Base::WaitForDialog(title);
	}

	static INPUT ki(WORD wVk, DWORD dwFlags = 0)
	{
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = wVk;
		input.ki.dwFlags = dwFlags;
		return input;
	};

	void EmulateHotKey() const
	{
		// ホットキー押下をエミュレートする
		std::vector<INPUT> inputs;
		inputs.emplace_back(ki(VK_CONTROL));
		inputs.emplace_back(ki(VK_MENU));
		inputs.emplace_back(ki('Z'));
		inputs.emplace_back(ki('Z', KEYEVENTF_KEYUP));
		inputs.emplace_back(ki(VK_MENU, KEYEVENTF_KEYUP));
		inputs.emplace_back(ki(VK_CONTROL, KEYEVENTF_KEYUP));
		SendInput(UINT(inputs.size()), inputs.data(), sizeof(INPUT));
	}
};

/*!
 * @brief 左コンテキストメニュー 新規作成 のテスト
 */
TEST_F(TrayIconTest, OpenNewEditor)
{
	ExecTrayCommand(WM_LBUTTONUP, F_FILENEW);	// 新規作成(&N)

	// 編集ウインドウが開くのを待つ
	const auto hWndEditor = WaitForEditor();

	// 編集ウインドウを閉じる。
	WaitForClose(hWndEditor, [this, hWndEditor] () {
		SendEditorCommand(hWndEditor, F_WINCLOSE);
	});
}

/*!
 * @brief 左コンテキストメニュー 開く のテスト
 */
TEST_F(TrayIconTest, OpenFile)
{
	// ファイルを開くのテスト
	const auto hWndDlgOpenFileName = WaitForTrayDialog(WM_LBUTTONUP, F_FILEOPEN, L"開く");

	// ファイル名を指定して開く
	EmulateSetOpenFileName(hWndDlgOpenFileName, gm_TestFilePath1);

	// 編集ウインドウが開くのを待つ
	const auto hWndEditor = WaitForEditor(BuildCaption(gm_TestFilePath1.wstring()));

	// 編集ウインドウを閉じる。
	WaitForClose(hWndEditor, [this, hWndEditor] () {
		SendEditorCommand(hWndEditor, F_WINCLOSE);
	});
}

/*!
 * @brief 左コンテキストメニュー Grep のテスト
 */
TEST_F(TrayIconTest, DoGrep)
{
	// Grepダイアログを開く
	const auto hWndDlgGrep = WaitForTrayDialog(WM_LBUTTONUP, F_GREP_DIALOG, L"Grep");

	const auto& szText = L"^";

	// 検索条件を入力
	auto pEditCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_EditControlTypeId);
	auto pFind = GetItemAt(hWndDlgGrep, TreeScope_Subtree, pEditCondition, 0);
	EmulateSetValue(pFind, szText);

	// Grep対象フォルダを入力
	auto pFolder = GetItemAt(hWndDlgGrep, TreeScope_Subtree, pEditCondition, 1);
	EmulateSetValue(pFolder, gm_TestFilePath1.parent_path().wstring());

	// ボタンを押下
	WaitForClose(hWndDlgGrep, [this, hWndDlgGrep] () {
		EmulateInvokeButton(hWndDlgGrep, L"検索(F)");
	});

	// Grep実行中ダイアログを待つ
	const auto hWndDlgGrepRunning = WaitForDialog(L"Grep実行中");
	EXPECT_THAT(hWndDlgGrepRunning, Ne(nullptr));

	// Grep実行中ダイアログを閉じる
	WaitForClose(hWndDlgGrepRunning, [] () {});

	// Grep結果ウインドウが開くのを待つ
	const auto hWndEditor = WaitForEditor(BuildCaption(szText));

	// 編集ウインドウを閉じる。
	WaitForClose(hWndEditor, [this, hWndEditor] () {
		SendEditorCommand(hWndEditor, F_WINCLOSE);
	});
}

/*!
 * @brief 左コンテキストメニュー 履歴の管理 のテスト
 */
TEST_F(TrayIconTest, ShowDlgFavarite)
{
	// 履歴とお気に入りの管理ダイアログを開く
	const auto hWndDlgFavorite = WaitForTrayDialog(WM_LBUTTONUP, F_FAVORITE, L"履歴とお気に入りの管理");

	WaitForClose(hWndDlgFavorite, [this, hWndDlgFavorite] () {
		EmulateInvokeButton(hWndDlgFavorite, L"閉じる(C)");
	});
}

/*!
 * @brief 右コンテキストメニュー タイプ別設定一覧 のテスト
 */
TEST_F(TrayIconTest, ShowDlgTypeList)
{
	// タイプ別設定一覧ダイアログを開く
	const auto hWndDlgTypeList = WaitForTrayDialog(WM_RBUTTONUP, F_TYPE_LIST, L"タイプ別設定一覧");

	// タイプ別設定ダイアログを開く
	WaitForClose(hWndDlgTypeList, [this, hWndDlgTypeList] () {
		EmulateInvokeButton(hWndDlgTypeList, L"設定変更(S)...");
	});

	// タイプ別設定プロパティシートを待つ
	const auto hWndPropTypes = WaitForDialog(L"タイプ別設定");
	EXPECT_THAT(hWndPropTypes, Ne(nullptr));

	// タイプ別設定プロパティシートを閉じる
	WaitForClose(hWndPropTypes, [this, hWndPropTypes] () {
		EmulateInvokeButton(hWndPropTypes, L"OK");
	});
}

/*!
 * @brief 右コンテキストメニュー 共通設定 の表示テスト
 */
TEST_F(TrayIconTest, ShowPropCommon)
{
	// 共通設定を開く
	const auto hWndPropCommon = WaitForTrayDialog(WM_RBUTTONUP, F_OPTION, L"共通設定");

	WaitForClose(hWndPropCommon, [this, hWndPropCommon] () {
		EmulateInvokeButton(hWndPropCommon, L"OK");
	});
}

/*!
 * @brief 右コンテキストメニュー バージョン情報 の表示テスト
 */
TEST_F(TrayIconTest, ShowDlgAbout)
{
	// バージョン情報ダイアログを開く
	const auto hWndDlgAbout = WaitForTrayDialog(WM_RBUTTONUP, F_ABOUT, L"バージョン情報");

	EmulateInvokeButton(hWndDlgAbout, L"情報をコピー(C)");

	WaitForClose(hWndDlgAbout, [this, hWndDlgAbout] () {
		EmulateInvokeButton(hWndDlgAbout, L"OK");
	});
}

/*!
 * @brief ダブルクリックで新規作成のテスト
 */
TEST_F(TrayIconTest, OpenNewEditorByDoubleClick)
{
	std::thread t([this] {
		// 編集ウインドウが開くのを待つ
		const auto hWndEditor = WaitForEditor();

		// 編集ウインドウを閉じる。
		WaitForClose(hWndEditor, [this, hWndEditor] () {
			SendEditorCommand(hWndEditor, F_WINCLOSE);
		});
	});

	// トレイアイコンのダブルクリックをエミュレートする
	SendMessageTimeoutW(hWndTray, MYWM_NOTIFYICON, 0, WM_LBUTTONDBLCLK,
		SMTO_BLOCK
		| SMTO_ABORTIFHUNG
		| SMTO_ERRORONEXIT
		,
		Base::defaultTimeoutMillis / 4,
		nullptr
	);

	t.join();
}

/*!
 * @brief ホットキーのテスト
 */
TEST_F(TrayIconTest, fireHotKey001)
{
	HWND hWndContextMenu = nullptr;
	do
	{
		// ホットキー押下をエミュレートする
		EmulateHotKey();

		// コンテキストメニューを待つ
		hWndContextMenu = Base::WaitForContextMenu();
	}
	while (!hWndContextMenu);

	WaitForClose(hWndTray, [this, hWndContextMenu] () {
		const auto index = GetContextMenuIndex(WM_LBUTTONUP, F_EXITALL);
		EmulateInvokeMenuItem(hWndContextMenu, index);
	});
}

/*!
 * @brief ホットキーのテスト
 *
 * 共通設定を開いているときは無効。
 */
TEST_F(TrayIconTest, fireHotKey101)
{
	// 共通設定を開く
	const auto hWndPropCommon = WaitForTrayDialog(WM_RBUTTONUP, F_OPTION, L"共通設定");

	// 非同期でコンテキストメニューを待機する
	std::thread t([] {
		const auto startTick = GetTickCount64();
		while (GetTickCount64() - startTick < defaultTimeoutMillis) {
			const auto hWndContextMenu = FindWindowW(MAKEINTRESOURCE(32768), nullptr);
			EXPECT_FALSE(hWndContextMenu);
	
			Sleep(10);  // 10msスリープしてリトライ
		}
	});

	// ホットキー押下をエミュレートする
	const auto startTick = GetTickCount64();
	while (GetTickCount64() - startTick < defaultTimeoutMillis / 2) {
		EmulateHotKey();
	
		Sleep(100);
	}

	t.join();

	WaitForClose(hWndPropCommon, [this, hWndPropCommon] () {
		EmulateInvokeButton(hWndPropCommon, L"OK");
	});
}

/*!
 * エディター機能テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
class EditorFuncTest : public testing::TSakuraGuiAware<::testing::Test> {
public:
	using Base = testing::TSakuraGuiAware<::testing::Test>;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	 static void SetUpTestSuite() {
		// OLEを初期化する
		EXPECT_TRUE(SetUpGuiTestSuite());

		// OLEの初期化に成功した場合
		if (gm_OleInitialized) {
			// 機能テストを初期化する
			SetUpFuncTest();

			// コントロールプロセスを起動する
			StartControlProcess();

			// コントロールプロセスに終了指示を出して終了を待つ
			TerminateControlProcess();

			// コントロールプロセスが終了すると、INIファイルが作成される
			EXPECT_TRUE(fexist(gm_IniPath));

			// 一時ファイル名を生成する
			// zipファイルパスの拡張子はzipにしないと動かない。
			auto tempPath = GetTempFilePath(L"tes", L"zip");

			// リソースからzipファイルデータを抽出して一時ファイルに書き込む
			const auto bin = CopyBinaryFromResource(IDR_ZIPRES2, RT_ZIPRES);
			EXPECT_FALSE(bin.empty());
			EXPECT_TRUE(WriteBinaryToFile(bin, tempPath));
			EXPECT_TRUE(std::filesystem::exists(tempPath));

			// インスタンス作成時にOLEが初期化されていればIsOkはtrueを返す
			auto destPath = gm_IniPath;
			destPath.remove_filename();
			destPath /= L"plugins";
			destPath /= L"";
			std::filesystem::create_directories(destPath);

			// zipファイルを解凍する
			CZipFile cZipFile;
			EXPECT_TRUE(cZipFile.IsOk());
			EXPECT_TRUE(cZipFile.SetZip(tempPath));
			EXPECT_TRUE(cZipFile.Unzip(destPath));

			// プラグインを有効にして保存。（UI操作で実施すると再起動が必要なため。）
			CDataProfile cProfile;
			cProfile.ReadProfile(gm_IniPath.c_str());
			cProfile.SetProfileData(L"Plugin", L"EnablePlugin", true);	// プラグインを使用する
			cProfile.SetProfileData(L"Plugin", L"P[00].CmdNum", 0);
			cProfile.SetProfileData(L"Plugin", L"P[00].Id", L"net.sourceforge.sakura-editor.plugin.Indent.Python"s);
			cProfile.SetProfileData(L"Plugin", L"P[00].Name", L"PythonIndent"s);
			cProfile.WriteProfile(gm_IniPath.c_str(), L"sakura.ini テキストエディタ設定ファイル");

			if (fexist(L"diff.exe"))		std::filesystem::remove(L"diff.exe");
			if (fexist(L"libiconv2.dll"))	std::filesystem::remove(L"libiconv2.dll");
			if (fexist(L"libintl3.dll"))	std::filesystem::remove(L"libintl3.dll");

			EXPECT_TRUE(WriteBinaryToFile(CopyBinaryFromResource(IDR_ZIPRES3, RT_ZIPRES), L"diff.exe"));
			EXPECT_TRUE(WriteBinaryToFile(CopyBinaryFromResource(IDR_ZIPRES4, RT_ZIPRES), L"libiconv2.dll"));
			EXPECT_TRUE(WriteBinaryToFile(CopyBinaryFromResource(IDR_ZIPRES5, RT_ZIPRES), L"libintl3.dll"));

			// テストファイル名を取得
			const auto filePath(gm_TestFilePath1.wstring());

			// エディタープロセスを起動する
			StartEditorProcess(filePath);
		}
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// コントロールプロセスに終了指示を出して終了を待つ
		TerminateControlProcess();

		if (fexist(L"カスタムメニュー.mnu"))	std::filesystem::remove(L"カスタムメニュー.mnu");
		if (fexist(L"キー割り当て.key"))		std::filesystem::remove(L"キー割り当て.key");
		if (fexist(L"テキスト.col"))			std::filesystem::remove(L"テキスト.col");
		if (fexist(L"テキスト.rkw"))			std::filesystem::remove(L"テキスト.rkw");
		if (fexist(L"テキスト.txt"))			std::filesystem::remove(L"テキスト.txt");
		if (fexist(L"メインメニュー.ini"))		std::filesystem::remove(L"メインメニュー.ini");
		if (fexist(L"強調キーワード.kwd"))		std::filesystem::remove(L"強調キーワード.kwd");

		if (fexist(L"diff.exe"))				std::filesystem::remove(L"diff.exe");
		if (fexist(L"libiconv2.dll"))			std::filesystem::remove(L"libiconv2.dll");
		if (fexist(L"libintl3.dll"))			std::filesystem::remove(L"libintl3.dll");

		// 機能テストをクリーンアップする
		TearDownFuncTest();

		// OLEの初期化を解除する
		TearDownGuiTestSuite();
	}

	/*!
	 * エディタプロセスを起動する
	 */
	static void StartEditorProcess(
		const std::filesystem::path& path
	)
	{
		StartSakuraProcess({ path.c_str() });
	}

	/*
	 * コンストラクタは流用
	 */
	using Base::Base;

	/*!
	 * 編集ウインドウのハンドル
	 */
	HWND hWndEditor = nullptr;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// UI Automationオブジェクトを作成する
		Base::SetUp();

		// テストファイル名を取得
		const auto filePath(gm_TestFilePath1.wstring());

		// 編集ウインドウのキャプションを組み立てる
		const auto caption = BuildCaption(filePath);

		// 編集ウインドウが開いているか確認する
		hWndEditor = FindWindowW(GSTR_EDITWINDOWNAME, caption.c_str());
		if (!hWndEditor || !IsWindow(hWndEditor)) {
			// エディタープロセスを起動する
			StartEditorProcess(filePath);

			// 編集ウインドウが開くのを待つ
			hWndEditor = WaitForEditor(caption);
		}
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// メッセージボックスが開いてないかチェック
		const auto hWndDlgMsgBox = FindWindowW(MAKEINTRESOURCE(32770), GSTR_APPNAME);
		EXPECT_FALSE(hWndDlgMsgBox);
	}

	void EmulateSetSaveFileName(
		_In_ HWND hWndDlgSaveFileName,
		const std::filesystem::path& path
	) const
	{
		auto pFileName = Base::GetFocusedElement();
		EmulateSetValue(pFileName, path.wstring());

		WaitForClose(hWndDlgSaveFileName, [this, hWndDlgSaveFileName] () {
			EmulateInvokeButton(hWndDlgSaveFileName, L"保存(S)");
		});
	}

	auto WaitForCommandDialog(
		EFunctionCode eFuncCd,
		std::wstring_view title
	) const
	{
		HWND hWndDlg = nullptr;

		std::thread t([&hWndDlg, this, title] {
			hWndDlg = WaitForDialog(title);
		});

		auto pEditor = Base::ElementFromHandle(hWndEditor);
		_com_util::CheckError(pEditor->SetFocus());

		SendEditorCommand(hWndEditor, eFuncCd);

		t.join();

		return hWndDlg;
	}
};

/*!
 * @brief ファイル内容比較ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgCompare001)
{
	if (fexist(gm_TestFilePath2)) {
		std::filesystem::remove(gm_TestFilePath2);
	}

	std::filesystem::copy_file(gm_TestFilePath1, gm_TestFilePath2, std::filesystem::copy_options::overwrite_existing);

	// テストファイル名を取得
	const auto filePath(gm_TestFilePath2.wstring());

	// エディタープロセスを起動する
	StartEditorProcess(filePath);

	// 編集ウインドウが開くのを待つ
	HWND hWndEditor2 = WaitForEditor(BuildCaption(filePath));

	auto pWndEditor = Base::ElementFromHandle(hWndEditor);
	_com_util::CheckError(pWndEditor->SetFocus());

	// ファイル内容比較ダイアログを開く
	if (const auto hWndDlgCompare = WaitForCommandDialog(F_COMPARE, L"ファイル内容比較")) {
		WaitForClose(hWndDlgCompare, [this, hWndDlgCompare] () {
			EmulateInvokeButton(hWndDlgCompare, L"OK");
		});
	}

	// 比較結果を待つ
	EXPECT_MSGBOX(hWndEditor, GSTR_APPNAME, L"異なる箇所は見つかりませんでした。");

	auto pWndEditor2 = Base::ElementFromHandle(hWndEditor2);
	_com_util::CheckError(pWndEditor2->SetFocus());

	WaitForClose(hWndEditor2, [this, hWndEditor2] () {
		SendEditorCommand(hWndEditor2, F_WINCLOSE);
	});

	_com_util::CheckError(pWndEditor->SetFocus());
}

/*!
 * @brief コントロールコードダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgCtrlCode001)
{
	const auto hWndDlgCtrlCode = WaitForCommandDialog(F_CTRL_CODE_DIALOG, L"コントロールコード");

	WaitForClose(hWndDlgCtrlCode, [this, hWndDlgCtrlCode] () {
		EmulateInvokeButton(hWndDlgCtrlCode, L"キャンセル(X)");
	});
}

/*!
 * @brief DIFF差分表示ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgDiff001)
{
	const auto hWndDlgDiff = WaitForCommandDialog(F_DIFF_DIALOG, L"DIFF差分表示");

	WaitForClose(hWndDlgDiff, [this, hWndDlgDiff] () {
		EmulateInvokeButton(hWndDlgDiff, L"キャンセル(X)");
	});
}

/*!
 * @brief DIFF差分表示ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgDiff002)
{
	if (fexist(gm_TestFilePath2)) {
		std::filesystem::remove(gm_TestFilePath2);
	}

	std::filesystem::copy_file(gm_TestFilePath1, gm_TestFilePath2, std::filesystem::copy_options::overwrite_existing);

	// テストファイル名を取得
	const auto filePath(gm_TestFilePath2.wstring());

	// エディタープロセスを起動する
	StartEditorProcess(filePath);

	// 編集ウインドウが開くのを待つ
	HWND hWndEditor2 = WaitForEditor(BuildCaption(filePath));

	auto pWndEditor = Base::ElementFromHandle(hWndEditor);
	_com_util::CheckError(pWndEditor->SetFocus());

	// DIFF差分表示ダイアログを開く
	if (const auto hWndDlgDiff = WaitForCommandDialog(F_DIFF_DIALOG, L"DIFF差分表示")) {
		WaitForClose(hWndDlgDiff, [this, hWndDlgDiff] () {
			EmulateInvokeButton(hWndDlgDiff, L"OK");
		});
	}

	// 終了を検出できないので長めに待つ
	Sleep(1000);

	auto pWndEditor2 = Base::ElementFromHandle(hWndEditor2);
	_com_util::CheckError(pWndEditor2->SetFocus());

	WaitForClose(hWndEditor2, [this, hWndEditor2] () {
		SendEditorCommand(hWndEditor2, F_WINCLOSE);
	});

	_com_util::CheckError(pWndEditor->SetFocus());
}

/*!
 * @brief ファイル名を指定して実行ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgExec001)
{
	const auto hWndDlgExec = WaitForCommandDialog(F_EXECMD_DIALOG, L"ファイル名を指定して実行");

	WaitForClose(hWndDlgExec, [this, hWndDlgExec] () {
		EmulateInvokeButton(hWndDlgExec, L"キャンセル(X)");
	});
}

/*!
 * @brief ファイル名を指定して実行ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgExec002)
{
	const auto hWndDlgExec = WaitForCommandDialog(F_EXECMD_DIALOG, L"ファイル名を指定して実行");

	// コマンドを入力
	auto pFileName = Base::GetFocusedElement();
	EmulateSetValue(pFileName, L"ctags.exe --help");

	WaitForClose(hWndDlgExec, [this, hWndDlgExec] () {
		EmulateInvokeButton(hWndDlgExec, L"実行(E)");
	});

	// コマンド実行中ダイアログを待つ
	const auto hWndDlgCmdRunning = WaitForDialog(L"コマンド実行中・・・");
	EXPECT_THAT(hWndDlgCmdRunning, Ne(nullptr));

	// コマンド実行中ダイアログを閉じる
	WaitForClose(hWndDlgCmdRunning, [] () {});

	// アウトプットウインドウが開くのを待つ
	const auto hWndOutput = WaitForEditor(BuildCaption(L"アウトプット(更新)"));
	EXPECT_THAT(hWndEditor, Ne(nullptr));

	WaitForClose(hWndOutput, [this, hWndOutput] () {
		SendEditorCommand(hWndOutput, F_WINCLOSE);
	});
}

/*!
 * @brief ファイルツリー設定ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgFileTree001)
{
	const auto hWndDlgFileTree1 = WaitForCommandDialog(F_FILETREE, L"ファイルツリー");

	EmulateInvokeButton(hWndDlgFileTree1, L"設定(S)");

	if (const auto hWndDlgFileTree2 = WaitForDialog(L"ファイルツリー設定")) {
		WaitForClose(hWndDlgFileTree2, [this, hWndDlgFileTree2] () {
			EmulateInvokeButton(hWndDlgFileTree2, L"キャンセル(X)");
		});
	}

	WaitForClose(hWndDlgFileTree1, [this, hWndDlgFileTree1] () {
		EmulateInvokeButton(hWndDlgFileTree1, L"キャンセル(X)");
	});

	WaitForClose(hWndEditor, [this] () {
		SendEditorCommand(hWndEditor, F_WINCLOSE);
	});
}

/*!
 * @brief ファイルが更新されましたダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgFileUpdateQuery001)
{
	std::atomic<bool> detected = false;

	// 非同期でタイムスタンプを更新する
	std::thread t([&detected] {
		const auto startTick = GetTickCount64();
		while (!detected && GetTickCount64() - startTick < defaultTimeoutMillis) {
			// ファイルの更新時刻を現在時刻に変更する
			std::filesystem::last_write_time(gm_TestFilePath1, std::filesystem::file_time_type::clock::now());
	
			Sleep(10);  // 10msスリープしてリトライ
		}
	});

	const auto hWndDlgFileUpdateQuery = WaitForDialog(L"ファイルが更新されました");

	detected = true;

	t.join();

	WaitForClose(hWndDlgFileUpdateQuery, [this, hWndDlgFileUpdateQuery] () {
		EmulateInvokeButton(hWndDlgFileUpdateQuery, L"閉じる(C)");
	});
}

/*!
 * @brief ファイルが更新されましたダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgFileUpdateQuery002)
{
	std::atomic<bool> detected = false;

	// 非同期でタイムスタンプを更新する
	std::thread t([&detected] {
		const auto startTick = GetTickCount64();
		while (!detected && GetTickCount64() - startTick < defaultTimeoutMillis) {
			// ファイルの更新時刻を現在時刻に変更する
			std::filesystem::last_write_time(gm_TestFilePath1, std::filesystem::file_time_type::clock::now());
	
			Sleep(10);  // 10msスリープしてリトライ
		}
	});

	const auto hWndDlgFileUpdateQuery = WaitForDialog(L"ファイルが更新されました");

	detected = true;

	t.join();

	WaitForClose(hWndDlgFileUpdateQuery, [this, hWndDlgFileUpdateQuery] () {
		EmulateInvokeButton(hWndDlgFileUpdateQuery, L"再読込(R)");
	});
}

/*!
 * @brief 履歴とお気に入りの管理ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgFavorite001)
{
	const auto hWndDlgFavorite = WaitForCommandDialog(F_FAVORITE, L"履歴とお気に入りの管理");

	WaitForClose(hWndDlgFavorite, [this, hWndDlgFavorite] () {
		EmulateInvokeButton(hWndDlgFavorite, L"閉じる(C)");
	});
}

/*!
 * @brief 検索ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgFind001)
{
	const auto hWndDlgFind = WaitForCommandDialog(F_SEARCH_DIALOG, L"検索");

	WaitForClose(hWndDlgFind, [this, hWndDlgFind] () {
		EmulateInvokeButton(hWndDlgFind, L"キャンセル(X)");
	});
}

/*!
 * @brief 検索ダイアログのテスト
 */
TEST_F(EditorFuncTest, DlgFind002)
{
	const auto hWndDlgFind = WaitForCommandDialog(F_SEARCH_DIALOG, L"検索");

	// 検索条件を入力
	auto pEditCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_EditControlTypeId);
	auto pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, L"条件(N)");
	auto pFinalCondition = CreateAndCondition(pEditCondition, pNameCondition);
	auto pFind = Base::FindFirst(hWndDlgFind, TreeScope_Subtree, pFinalCondition);
	EmulateSetValue(pFind, L"2");

	WaitForClose(hWndDlgFind, [this, hWndDlgFind] () {
		EmulateInvokeButton(hWndDlgFind, L"下検索(D)");
	});

	SendEditorCommand(hWndEditor, F_SEARCH_NEXT);

	SendEditorCommand(hWndEditor, F_SEARCH_PREV);
}

/*!
 * @brief 開くダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgGetOpenFileName001)
{
	const auto hWndDlgGetOpenFileName = WaitForCommandDialog(F_FILEOPEN, L"開く");

	WaitForClose(hWndDlgGetOpenFileName, [this, hWndDlgGetOpenFileName] () {
		EmulateInvokeButton(hWndDlgGetOpenFileName, L"キャンセル");
	});
}

/*!
 * @brief 名前を付けて保存ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgGetSaveFileName001)
{
	const auto hWndDlgSaveFileName = WaitForCommandDialog(F_FILESAVEAS_DIALOG, L"名前を付けて保存" );

	WaitForClose(hWndDlgSaveFileName, [this, hWndDlgSaveFileName] () {
		EmulateInvokeButton(hWndDlgSaveFileName, L"キャンセル");
	});
}

/*!
 * @brief 名前を付けて保存ダイアログの機能テスト
 */
TEST_F(EditorFuncTest, DlgGetSaveFileName002)
{
	if (fexist(gm_TestFilePath2)) {
		std::filesystem::remove(gm_TestFilePath2);
	}

	const auto hWndDlgSaveFileName = WaitForCommandDialog(F_FILESAVEAS_DIALOG, L"名前を付けて保存" );

	// 名前を付けて保存
	EmulateSetSaveFileName(hWndDlgSaveFileName, gm_TestFilePath2);

	// ファイル名が変わってしまったので編集ウインドウを閉じて終わる
	WaitForClose(hWndEditor, [this] () {
		SendEditorCommand(hWndEditor, F_WINCLOSE);
	});
}

/*!
 * @brief Grepダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgGrep001)
{
	const auto hWndDlgGrep = WaitForCommandDialog(F_GREP_DIALOG, L"Grep");

	WaitForClose(hWndDlgGrep, [this, hWndDlgGrep] () {
		EmulateInvokeButton(hWndDlgGrep, L"キャンセル(X)");
	});
}

/*!
 * @brief Grep置換ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgGrepReplace001)
{
	const auto hWndDlgGrepReplace = WaitForCommandDialog(F_GREP_REPLACE_DLG, L"Grep置換");

	WaitForClose(hWndDlgGrepReplace, [this, hWndDlgGrepReplace] () {
		EmulateInvokeButton(hWndDlgGrepReplace, L"キャンセル(X)");
	});
}

/*!
 * @brief 指定行へジャンプダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgJump001)
{
	const auto hWndDlgJump = WaitForCommandDialog(F_JUMP_DIALOG, L"指定行へジャンプ");

	WaitForClose(hWndDlgJump, [this, hWndDlgJump] () {
		EmulateInvokeButton(hWndDlgJump, L"キャンセル(X)");
	});
}

/*!
 * @brief プラグイン設定ダイアログのテスト
 */
TEST_F(EditorFuncTest, DlgPluginOption)
{
	SendEditorCommand(hWndEditor, F_OPTION);

	IUIAutomationConditionPtr pNameCondition;
	IUIAutomationConditionPtr pFinalCondition;

	// 共通設定のテスト
	const auto hWndPropCommon = WaitForDialog(L"共通設定");

	// タブアイテムの列挙条件を作成
	auto pTabItemCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_TabItemControlTypeId);
	pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, L"プラグイン");
	pFinalCondition = CreateAndCondition(pTabItemCondition, pNameCondition);
	auto pTabItem = Base::FindFirst(hWndPropCommon, TreeScope_Subtree, pFinalCondition);

	// タブを選択
	EmulateSelectItem(pTabItem);

	// リスト項目を選択する
	auto pListItemCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_ListItemControlTypeId);
	pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, L"0");
	pFinalCondition = CreateAndCondition(pListItemCondition, pNameCondition);
	auto pListItem = Base::FindFirst(hWndPropCommon, TreeScope_Subtree, pFinalCondition);
	EmulateSelectItem(pListItem);

	EmulateInvokeButton(hWndPropCommon, L"設定(P)");

	if (const auto hWndDlgPluginOption = WaitForDialog(L"Pythonスマートインデント プラグインの設定")) {
		WaitForClose(hWndDlgPluginOption, [this, hWndDlgPluginOption] () {
			EmulateInvokeButton(hWndDlgPluginOption, L"キャンセル(X)");
		});
	}

	WaitForClose(hWndPropCommon, [this, hWndPropCommon] () {
		EmulateInvokeButton(hWndPropCommon, L"OK");
	});
}

/*!
 * @brief 印刷ページ設定ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgPrintSetting001)
{
	const auto hWndDlgPrintSetting = WaitForCommandDialog(F_PRINT_PAGESETUP, L"印刷ページ設定");

	WaitForClose(hWndDlgPrintSetting, [this, hWndDlgPrintSetting] () {
		EmulateInvokeButton(hWndDlgPrintSetting, L"キャンセル(X)");
	});
}

/*!
 * @brief プロファイルマネージャダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgProfileMgr001)
{
	const auto hWndDlgProfileMgr = WaitForCommandDialog(F_PROFILEMGR, L"プロファイルマネージャ");

	WaitForClose(hWndDlgProfileMgr, [this, hWndDlgProfileMgr] () {
		EmulateInvokeButton(hWndDlgProfileMgr, L"閉じる(X)");
	});
}

/*!
 * @brief ファイルのプロパティダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgProperty001)
{
	const auto hWndDlgProperty = WaitForCommandDialog(F_PROPERTY_FILE, L"ファイルのプロパティ");

	WaitForClose(hWndDlgProperty, [this, hWndDlgProperty] () {
		EmulateInvokeButton(hWndDlgProperty, L"閉じる(C)");
	});
}

/*!
 * @brief 置換ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgReplace001)
{
	const auto hWndDlgReplace = WaitForCommandDialog(F_REPLACE_DIALOG, L"置換");

	WaitForClose(hWndDlgReplace, [this, hWndDlgReplace] () {
		EmulateInvokeButton(hWndDlgReplace, L"キャンセル(X)");
	});
}

/*!
 * @brief 置換ダイアログのテスト
 */
TEST_F(EditorFuncTest, DlgReplace002)
{
	const auto hWndDlgReplace = WaitForCommandDialog(F_REPLACE_DIALOG, L"置換");

	// 検索条件を入力
	auto pEditCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_EditControlTypeId);
	auto pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, L"置換前(N)");
	auto pFinalCondition = CreateAndCondition(pEditCondition, pNameCondition);
	auto pFind = Base::FindFirst(hWndDlgReplace, TreeScope_Subtree, pFinalCondition);
	EmulateSetValue(pFind, L"3");

	pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, L"置換後(P)");
	pFinalCondition = CreateAndCondition(pEditCondition, pNameCondition);
	auto pRep = Base::FindFirst(hWndDlgReplace, TreeScope_Subtree, pFinalCondition);
	EmulateSetValue(pRep, L"さぁ～～～ん！");

	EmulateInvokeButton(hWndDlgReplace, L"置換(R)");

	WaitForClose(hWndDlgReplace, [this, hWndDlgReplace] () {
		EmulateInvokeButton(hWndDlgReplace, L"キャンセル(X)");
	});

	SendEditorCommand(hWndEditor, F_UNDO);

	SendEditorCommand(hWndEditor, F_REDO);

	WaitForClose(hWndEditor, [this] () {
		EXPECT_MSGBOX2(SendEditorCommand(hWndEditor, F_WINCLOSE), GSTR_APPNAME, std::nullopt, L"いいえ(N)");
	});
}

/*!
 * @brief 置換ダイアログのテスト
 */
TEST_F(EditorFuncTest, DlgReplace003)
{
	const auto hWndDlgReplace = WaitForCommandDialog(F_REPLACE_DIALOG, L"置換");

	// 検索条件を入力
	auto pEditCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_EditControlTypeId);
	auto pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, L"置換前(N)");
	auto pFinalCondition = CreateAndCondition(pEditCondition, pNameCondition);
	auto pFind = Base::FindFirst(hWndDlgReplace, TreeScope_Subtree, pFinalCondition);
	EmulateSetValue(pFind, L"3");

	pNameCondition = CreatePropertyCondition(UIA_NamePropertyId, L"置換後(P)");
	pFinalCondition = CreateAndCondition(pEditCondition, pNameCondition);
	auto pRep = Base::FindFirst(hWndDlgReplace, TreeScope_Subtree, pFinalCondition);
	EmulateSetValue(pRep, L"さぁ～～～ん！");

	WaitForClose(hWndDlgReplace, [this, hWndDlgReplace] () {
		EXPECT_MSGBOX(EmulateInvokeButton(hWndDlgReplace, L"すべて置換(A)"), GSTR_APPNAME, std::nullopt);
	});

	WaitForClose(hWndEditor, [this] () {
		EXPECT_MSGBOX2(SendEditorCommand(hWndEditor, F_WINCLOSE), GSTR_APPNAME, std::nullopt, L"いいえ(N)");
	});
}

/*!
 * @brief 文字コードの指定ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgSetCharSet001)
{
	const auto hWndDlgSetCharSet = WaitForCommandDialog(F_CHG_CHARSET, L"文字コードの指定");

	WaitForClose(hWndDlgSetCharSet, [this, hWndDlgSetCharSet] () {
		EmulateInvokeButton(hWndDlgSetCharSet, L"キャンセル(X)");
	});
}

/*!
 * @brief ダイレクトタグジャンプ一覧ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgTagJumpList001)
{
	const auto hWndDlgTagJumpList = WaitForCommandDialog(F_TAGJUMP_KEYWORD, L"ダイレクトタグジャンプ一覧");

	WaitForClose(hWndDlgTagJumpList, [this, hWndDlgTagJumpList] () {
		EmulateInvokeButton(hWndDlgTagJumpList, L"キャンセル(X)");
	});
}

/*!
 * @brief タグファイルの作成ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgTagsMake001)
{
	const auto hWndDlgTagsMake = WaitForCommandDialog(F_TAGS_MAKE, L"タグファイルの作成");

	WaitForClose(hWndDlgTagsMake, [this, hWndDlgTagsMake] () {
		EmulateInvokeButton(hWndDlgTagsMake, L"キャンセル(X)");
	});
}

/*!
 * @brief タイプ別設定一覧ダイアログのテスト
 */
TEST_F(EditorFuncTest, DlgTypeList)
{
	// タイプ別設定一覧ダイアログ
	const auto hWndDlgTypeList = WaitForCommandDialog(F_TYPE_LIST, L"タイプ別設定一覧");

	WaitForClose(hWndDlgTypeList, [this, hWndDlgTypeList] () {
		EmulateInvokeButton(hWndDlgTypeList, L"キャンセル(X)");
	});
}

/*!
 * @brief インポート確認ダイアログのテスト
 */
TEST_F(EditorFuncTest, DlgTypeAscertain)
{
	if (fexist(gm_ExportedTypeFilePath)) {
		std::filesystem::remove(gm_ExportedTypeFilePath);
	}

	// タイプ別設定一覧ダイアログ
	const auto hWndDlgTypeList = WaitForCommandDialog(F_TYPE_LIST, L"タイプ別設定一覧");

	EmulateInvokeButton(hWndDlgTypeList, L"エクスポート(E)");

	if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
		// エクスポート実行
		EmulateSetSaveFileName(hWndDlgSaveFileName, gm_ExportedTypeFilePath.filename());
	}

	EXPECT_MSGBOX(hWndDlgTypeList, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", gm_ExportedTypeFilePath.c_str()));

	EmulateInvokeButton(hWndDlgTypeList, L"インポート(I)");

	if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
		// インポート実行
		EmulateSetOpenFileName(hWndDlgOpenFileName, gm_ExportedTypeFilePath);
	}

	if (const auto hWndDlgTypeAscertain = WaitForDialog(L"インポート確認")) {
		WaitForClose(hWndDlgTypeAscertain, [this, hWndDlgTypeAscertain] () {
			EmulateInvokeButton(hWndDlgTypeAscertain, L"OK");
		});
	}

	EXPECT_MSGBOX(hWndDlgTypeList, GSTR_APPNAME, strprintf(L"ファイルをインポートしました。\n\n%s", gm_ExportedTypeFilePath.c_str()));

	EmulateInvokeButton(hWndDlgTypeList, L"↑(U)");

	EmulateInvokeButton(hWndDlgTypeList, L"↓(O)");

	EmulateInvokeButton(hWndDlgTypeList, L"複製(C)");

	EXPECT_MSGBOX(EmulateInvokeButton(hWndDlgTypeList, L"削除(D)"), GSTR_APPNAME, std::nullopt);

	EXPECT_MSGBOX(EmulateInvokeButton(hWndDlgTypeList, L"削除(D)"), GSTR_APPNAME, std::nullopt);

	WaitForClose(hWndDlgTypeList, [this, hWndDlgTypeList] () {
		EmulateInvokeButton(hWndDlgTypeList, L"キャンセル(X)");
	});
}

/*!
 * @brief ウィンドウ一覧ダイアログの表示テスト
 */
TEST_F(EditorFuncTest, DlgWindowList001)
{
	const auto hWndDlgWindowList = WaitForCommandDialog(F_DLGWINLIST, L"ウィンドウ一覧");

	WaitForClose(hWndDlgWindowList, [this, hWndDlgWindowList] () {
		EmulateInvokeButton(hWndDlgWindowList, L"OK");
	});
}

/*!
 * @brief インデント機能のテスト
 */
TEST_F(EditorFuncTest, FuncIndent001)
{
	SendEditorCommand(hWndEditor, F_INDENT_TAB);

	SendEditorCommand(hWndEditor, F_UNINDENT_TAB);

	WaitForClose(hWndEditor, [this] () {
		EXPECT_MSGBOX2(SendEditorCommand(hWndEditor, F_WINCLOSE), GSTR_APPNAME, std::nullopt, L"いいえ(N)");
	});
}

/*!
 * @brief インデント機能のテスト
 */
TEST_F(EditorFuncTest, FuncIndent002)
{
	SendEditorCommand(hWndEditor, F_INDENT_SPACE);

	SendEditorCommand(hWndEditor, F_UNINDENT_SPACE);

	WaitForClose(hWndEditor, [this] () {
		EXPECT_MSGBOX2(SendEditorCommand(hWndEditor, F_WINCLOSE), GSTR_APPNAME, std::nullopt, L"いいえ(N)");
	});
}

/*!
 * @brief ソート機能のテスト
 */
TEST_F(EditorFuncTest, FuncSort001)
{
	// 行コピー＆貼り付けで先頭行を複製
	SendEditorCommand(hWndEditor, F_COPY);
	SendEditorCommand(hWndEditor, F_GOFILEEND);
	SendEditorCommand(hWndEditor, F_PASTE);

	// 行のソート（昇順）
	SendEditorCommand(hWndEditor, F_SELECTALL);
	SendEditorCommand(hWndEditor, F_SORT_ASC);

	// 重複行をマージ（メッセージは無視する）
	EXPECT_MSGBOX(SendEditorCommand(hWndEditor, F_MERGE), GSTR_APPNAME, std::nullopt);

	// 行のソート（昇順）
	SendEditorCommand(hWndEditor, F_SELECTALL);
	SendEditorCommand(hWndEditor, F_SORT_DESC);

	WaitForClose(hWndEditor, [this] () {
		EXPECT_MSGBOX2(SendEditorCommand(hWndEditor, F_WINCLOSE), GSTR_APPNAME, std::nullopt, L"いいえ(N)");
	});
}

/*!
 * @brief 共通設定プロパティーシートのテスト
 */
TEST_F(EditorFuncTest, PropCommon)
{
	// 共通設定のテスト
	const auto hWndPropCommon = WaitForCommandDialog(F_OPTION, L"共通設定");

	// タブアイテムの列挙条件を作成
	auto pTabItemCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_TabItemControlTypeId);
	// タブアイテムを列挙して選択
	ForEachItems(hWndPropCommon, pTabItemCondition, [&](IUIAutomationElement* pItem) {
		std::wstring tabName;
		if (BSTR bstrName = nullptr; SUCCEEDED(pItem->get_CurrentName(&bstrName)) && bstrName) {
			_bstr_t name(bstrName);
			TRACE("tab: %s\n", bstrName);
			tabName = bstrName;
		}

		// タブを選択
		EmulateSelectItem(pItem);

		if (L"ウィンドウ" == tabName) {
			EmulateInvokeButton(hWndPropCommon, L"位置と大きさの設定(W)...");

			if (const auto hWndDlgWinSize = WaitForDialog(L"ウィンドウの位置と大きさ")) {
				WaitForClose(hWndDlgWinSize, [this, hWndDlgWinSize] () {
					EmulateInvokeButton(hWndDlgWinSize, L"閉じる(C)");
				});
			}
		}
		if (L"メインメニュー" == tabName) {

			const auto exportedPath = std::filesystem::current_path() / L"メインメニュー.ini";

			if (fexist(exportedPath)) {
				std::filesystem::remove(exportedPath);
			}

			EmulateInvokeButton(hWndPropCommon, L"エクスポート(X)...");

			if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
				// エクスポート実行
				EmulateSetSaveFileName(hWndDlgSaveFileName, exportedPath);
			}

			EXPECT_MSGBOX(hWndPropCommon, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", exportedPath.c_str()));

			EmulateInvokeButton(hWndPropCommon, L"インポート(I)...");

			if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
				// インポート実行
				EmulateSetOpenFileName(hWndDlgOpenFileName, exportedPath.filename());
			}
		}
		if (L"カスタムメニュー" == tabName) {

			const auto exportedPath = std::filesystem::current_path() / L"カスタムメニュー.mnu";

			if (fexist(exportedPath)) {
				std::filesystem::remove(exportedPath);
			}

			EmulateInvokeButton(hWndPropCommon, L"エクスポート(X)...");

			if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
				// エクスポート実行
				EmulateSetSaveFileName(hWndDlgSaveFileName, exportedPath);
			}

			EXPECT_MSGBOX(hWndPropCommon, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", exportedPath.c_str()));

			EmulateInvokeButton(hWndPropCommon, L"インポート(I)...");

			if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
				// インポート実行
				EmulateSetOpenFileName(hWndDlgOpenFileName, exportedPath.filename());
			}
		}
		if (L"強調キーワード" == tabName) {
			EmulateInvokeButton(hWndPropCommon, L"セット追加(M)...");

			if (const auto hWndDlgWinSize = WaitForDialog(L"キーワードのセット追加")) {
				WaitForClose(hWndDlgWinSize, [this, hWndDlgWinSize] () {
					EmulateInvokeButton(hWndDlgWinSize, L"キャンセル(X)");
				});
			}

			const auto exportedPath = std::filesystem::current_path() / L"強調キーワード.kwd";

			if (fexist(exportedPath)) {
				std::filesystem::remove(exportedPath);
			}

			EmulateInvokeButton(hWndPropCommon, L"エクスポート(X)...");

			if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
				// エクスポート実行
				EmulateSetSaveFileName(hWndDlgSaveFileName, exportedPath);
			}

			EXPECT_MSGBOX(hWndPropCommon, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", exportedPath.c_str()));

			EmulateInvokeButton(hWndPropCommon, L"インポート(I)...");

			if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
				// インポート実行
				EmulateSetOpenFileName(hWndDlgOpenFileName, exportedPath.filename());
			}
		}
		if (L"キー割り当て" == tabName) {

			const auto exportedPath = std::filesystem::current_path() / L"キー割り当て.key";

			if (fexist(exportedPath)) {
				std::filesystem::remove(exportedPath);
			}

			EmulateInvokeButton(hWndPropCommon, L"エクスポート(X)...");

			if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
				// エクスポート実行
				EmulateSetSaveFileName(hWndDlgSaveFileName, exportedPath);
			}

			EXPECT_MSGBOX(hWndPropCommon, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", exportedPath.c_str()));

			EmulateInvokeButton(hWndPropCommon, L"インポート(I)...");

			if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
				// インポート実行
				EmulateSetOpenFileName(hWndDlgOpenFileName, exportedPath.filename());
			}
		}
	});

	WaitForClose(hWndPropCommon, [this, hWndPropCommon] () {
		EmulateInvokeButton(hWndPropCommon, L"OK");
	});
}

/*!
 * @brief タイプ別設定プロパティーシートのテスト
 */
TEST_F(EditorFuncTest, PropTypes)
{
	// タイプ別設定プロパティシートを待ってEnterキー押下
	const auto hWndPropTypes = WaitForCommandDialog(F_OPTION_TYPE, L"タイプ別設定");

	// タブアイテムの列挙条件を作成
	auto pTabItemCondition = CreatePropertyCondition(UIA_ControlTypePropertyId, UIA_TabItemControlTypeId);
	// タブアイテムを列挙して選択
	ForEachItems(hWndPropTypes, pTabItemCondition, [&](IUIAutomationElement* pItem) {
		std::wstring tabName;
		if (BSTR bstrName = nullptr; SUCCEEDED(pItem->get_CurrentName(&bstrName)) && bstrName) {
			_bstr_t name(bstrName);
			TRACE("tab: %s\n", bstrName);
			tabName = bstrName;
		}

		// タブを選択
		EmulateSelectItem(pItem);

		if (L"ウィンドウ" == tabName) {
			EmulateInvokeButton(hWndPropTypes, L"...");

			if (const auto hWndDlgGetOpenFileName = WaitForDialog(L"開く")) {
				WaitForClose(hWndDlgGetOpenFileName, [this, hWndDlgGetOpenFileName] () {
					EmulateInvokeButton(hWndDlgGetOpenFileName, L"キャンセル");
				});
			}
		}
		if (L"カラー" == tabName) {
			EmulateInvokeButton(hWndPropTypes, L"文字色統一(<)...");

			if (const auto hWndDlgWinSize1 = WaitForDialog(L"文字色統一")) {
				WaitForClose(hWndDlgWinSize1, [this, hWndDlgWinSize1] () {
					EmulateInvokeButton(hWndDlgWinSize1, L"キャンセル(X)");
				});
			}

			EmulateInvokeButton(hWndPropTypes, L"背景色統一(>)...");

			if (const auto hWndDlgWinSize2 = WaitForDialog(L"背景色統一")) {
				WaitForClose(hWndDlgWinSize2, [this, hWndDlgWinSize2] () {
					EmulateInvokeButton(hWndDlgWinSize2, L"キャンセル(X)");
				});
			}

			EmulateInvokeButton(hWndPropTypes, L"2～10...");

			if (const auto hWndDlgKeywordSelect = WaitForDialog(L"強調キーワードの設定")) {
				WaitForClose(hWndDlgKeywordSelect, [this, hWndDlgKeywordSelect] () {
					EmulateInvokeButton(hWndDlgKeywordSelect, L"キャンセル(X)");
				});
			}

			const auto exportedPath = std::filesystem::current_path() / L"テキスト.col";

			if (fexist(exportedPath)) {
				std::filesystem::remove(exportedPath);
			}

			EmulateInvokeButton(hWndPropTypes, L"エクスポート(X)...");

			if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
				// エクスポート実行
				EmulateSetSaveFileName(hWndDlgSaveFileName, exportedPath);
			}

			EXPECT_MSGBOX(hWndPropTypes, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", exportedPath.c_str()));

			EmulateInvokeButton(hWndPropTypes, L"インポート(I)...");

			if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
				// インポート実行
				EmulateSetOpenFileName(hWndDlgOpenFileName, exportedPath.filename());
			}
		}
		if (L"正規表現キーワード" == tabName) {

			const auto exportedPath = std::filesystem::current_path() / L"テキスト.rkw";

			if (fexist(exportedPath)) {
				std::filesystem::remove(exportedPath);
			}

			EmulateInvokeButton(hWndPropTypes, L"エクスポート(X)...");

			if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
				// エクスポート実行
				EmulateSetSaveFileName(hWndDlgSaveFileName, exportedPath);
			}

			EXPECT_MSGBOX(hWndPropTypes, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", exportedPath.c_str()));

			EmulateInvokeButton(hWndPropTypes, L"インポート(I)...");

			if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
				// インポート実行
				EmulateSetOpenFileName(hWndDlgOpenFileName, exportedPath.filename());
			}
		}
		if (L"キーワードヘルプ" == tabName) {

			const auto exportedPath = std::filesystem::current_path() / L"テキスト.txt";

			if (fexist(exportedPath)) {
				std::filesystem::remove(exportedPath);
			}

			EmulateInvokeButton(hWndPropTypes, L"エクスポート(X)...");

			if (const auto hWndDlgSaveFileName = WaitForDialog(L"名前を付けて保存")) {
				// エクスポート実行
				EmulateSetSaveFileName(hWndDlgSaveFileName, exportedPath);
			}

			EXPECT_MSGBOX(hWndPropTypes, GSTR_APPNAME, strprintf(L"ファイルをエクスポートしました。\n\n%s", exportedPath.c_str()));

			EmulateInvokeButton(hWndPropTypes, L"インポート(I)...");

			if (const auto hWndDlgOpenFileName = WaitForDialog(L"開く")) {
				// インポート実行
				EmulateSetOpenFileName(hWndDlgOpenFileName, exportedPath.filename());
			}
		}
	});

	WaitForClose(hWndPropTypes, [this, hWndPropTypes] () {
		EmulateInvokeButton(hWndPropTypes, L"OK");
	});
}

} // namespace func
