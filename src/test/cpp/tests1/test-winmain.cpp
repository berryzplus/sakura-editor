/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <fstream>

#include "config/maxdata.h"
#include "basis/primitive.h"
#include "debug/Debug2.h"
#include "basis/CMyString.h"
#include "mem/CNativeW.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "util/file.h"
#include "config/system_constants.h"
#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"

#include "testing/StartEditorProcess.hpp"
#include "dlg/ModalDialogCloser.hpp"
#include "window/EditorTestSuite.hpp"

#include "tests1_rc.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

void extract_zip_resource(WORD id, const std::optional<std::filesystem::path>& optOutDir);

namespace cxx {

/*!
 * @brief テキストファイルを書き出す
 *
 * @param outPath 出力先パス
 * @param lines 書き込む行の配列
 * @note 使い物になるかどうか試作してみただけ
 */
void writeTextFile(
	const std::filesystem::path& outPath,
	std::span<const std::u8string_view> lines
)
{
	if (const auto parentPath = outPath.parent_path(); !fexist(parentPath)) {
		// ディレクトリが存在しない場合は作成する
		std::filesystem::create_directories(parentPath);
	}

	// ファイル出力ストリームをバイナリモードで開く
	std::ofstream fs(outPath, std::ios::binary);

	// UTF-8 BOMを出力
	const std::array bom = { '\xEF', '\xBB', '\xBF' };
	fs.write(bom.data(), bom.size());

	// 各行を書き込む
	for (const auto& line : lines) {
		if (!line.empty()) {
			fs.write(LPCSTR(std::data(line)), std::size(line));
		}
		fs << "\r\n";
	}

	fs.close();
}

/*!
 * @brief 起動したエディタープロセスオブジェクト
 *
 * @note 使い物になるかどうか試作してみた
 */
class EditorProcessHolder : public cxx::ProcessHolder
{
private:
	using Base = cxx::ProcessHolder;
	using Me = EditorProcessHolder;

public:
	explicit EditorProcessHolder(
		HANDLE hProcess,
		DWORD dwProcessId,
		DWORD dwThreadId,
		HWND hWnd
	)
		: Base(hProcess, dwProcessId, dwThreadId)
		, hWnd(hWnd)
	{
	}

	EditorProcessHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	EditorProcessHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	HWND hWnd;
};

} // namespace cxx

namespace testing {

/*!
 * @brief コントロールプロセスを起動する
 *
 * @param profileName プロファイル名
 * @return 起動したプロセスオブジェクト
 * @note 使い物になるかどうか試作してみた
 */
cxx::ProcessHolder CreateControlProcess(std::wstring_view profileName)
{
	return CProcess::CreateControlProcess(std::wstring{ profileName });
}

/*!
 * @brief プロセスを起動する
 *
 * @tparam T コマンドライン引数のコンテナ型
 * @param args コマンドライン引数
 * @param profileName プロファイル名
 * @return 起動したプロセスオブジェクト
 * @note 使い物になるかどうか試作してみた
 */
template<class T>
	requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
cxx::ProcessHolder CreateSakuraProcess(
	const T& args,
	std::wstring_view profileName
)
{
	// コマンドライン引数の編集用vector
	std::vector<std::wstring> commandArgs{ std::begin(args), std::end(args) };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	// エディタープロセスを起動する
	auto ep = CProcess::CreateSakuraProcess(si, commandArgs, std::nullopt, std::wstring{ profileName });
	EXPECT_THAT(ep, NotNull());

	// プロセスオブジェクトを返す
	return ep;
}

/*!
 * @brief 編集ウィンドウを列挙するコールバック関数
 *
 * @param hWnd 列挙されたウィンドウのハンドル
 * @param lParam 列挙の呼び出し元から渡されたパラメータ（HWND* を期待）
 * @retval TRUE 列挙続行（編集ウィンドウではなかった。）
 * @retval FALSE 列挙停止（編集ウィンドウが見付かった。）
 */
BOOL CALLBACK EnumEditorWindowProc(
	_In_ HWND   hWnd,
	_In_ LPARAM lParam
)
{
	if (!hWnd || !lParam || !IsSakuraMainWindow(hWnd)) return TRUE;	// 検索続行

	auto phWndFound = std::bit_cast<HWND*>(lParam);

	*phWndFound = hWnd;

	return FALSE;
}

/*!
 * @brief エディタープロセスを起動する
 *
 * @tparam T コマンドライン引数のコンテナ型
 * @param args コマンドライン引数
 * @param profileName プロファイル名
 * @return 起動したプロセスオブジェクト
 * @note 使い物になるかどうか試作してみた
 */
template<class T>
	requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
cxx::EditorProcessHolder CreateEditorProcess(
	const T& args,
	std::wstring_view profileName,
	const std::optional<std::filesystem::path>& optWorkingDir = std::nullopt
)
{
	// コマンドライン引数の編集用vector
	std::vector<std::wstring> commandArgs{ std::begin(args), std::end(args) };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	// エディタープロセスを起動する
	auto ep = CProcess::CreateSakuraProcess(si, commandArgs, std::nullopt, std::wstring{ profileName });
	EXPECT_THAT(ep, NotNull());

	// 初期化完了イベントを作成する
	SFilePath initEventName{ std::format(GSTR_EVENT_SAKURA_EP_INITIALIZED, ep.dwThreadId) };
	cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, initEventName);

	const auto startTick = ::GetTickCount64();

	// メインウインドウを取得する
	HWND hWndFound = nullptr;
	do {
		// スレッドに含まれるウインドウを列挙する
		::EnumThreadWindows(ep.dwThreadId, EnumEditorWindowProc, LPARAM(&hWndFound));

		if (hWndFound) break;

		Sleep(100);  // 100msスリープしてリトライ
	}
	while (::GetTickCount64() - startTick < 30000);

	EXPECT_THAT(hWndFound, NotNull());

	// 初期化完了を待つ
	hEvent.lock();

	// プロセスオブジェクトを返す
	return cxx::EditorProcessHolder{ ep.release(), ep.dwProcessId, ep.dwThreadId, hWndFound };
}

//! 外部ウインドウにクローズを要求する
void RequestForeignWindowClose(HWND hWnd)
{
	// ウインドウが閉じられるまで繰り返す
	while (::IsWindow(hWnd)) {
		// プロセス間通信なのでポストする
		::PostMessageW(hWnd, WM_CLOSE, 0, 0);

		// 少し待つ
		::Sleep(100);
	}
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 */
void TerminateControlProcess(
	std::wstring_view profileName,
	DWORD dwControlProcessId = 0
)
{
	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	if (const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName)) {
		if (!dwControlProcessId) {
			// トレイウインドウからプロセスIDを取得する
			::GetWindowThreadProcessId(hTrayWnd, &dwControlProcessId);
			if (!dwControlProcessId) {
				cxx::raise_system_error("dwControlProcessId can't be retrived.");
			}
		}

		// トレイウインドウにクローズを要求する
		RequestForeignWindowClose(hTrayWnd);
	}

	// プロセス情報の問い合せを行うためのハンドルを開く
	// タイムアウト時に強制終了へフォールバックできるよう、TERMINATE 権限も付与する
	cxx::HandleHolder process{ ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, dwControlProcessId) };
	if (!process) {
		// プロセスIDが無効は「既に終了している」なので、除外する
		if (ERROR_INVALID_PARAMETER == ::GetLastError()) {
			return;
		}
		cxx::raise_system_error("hControlProcess can't be opened.");
	}

	// メインウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	process.lock();
}

} // namespace testing

namespace cxx {

TEST(HandleHolder, Lock101)
{
	// ハンドルを開かずにロックを試み、失敗させる
	cxx::HandleHolder handle{};
	EXPECT_THAT(handle.Lock(), IsFalse());
}

TEST(raise_system_error, test01)
{
	EXPECT_THROW({ raise_system_error("test"); }, std::system_error);
}

} // namespace cxx

namespace winmain {

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
template<class T>
struct TWinMainTest : public T, public window::UiaTestSuite {
	using Base = T;

	/*!
	 * テスト用ファイルのパス
	 */
	static inline std::filesystem::path gm_TestDataPath = std::filesystem::current_path() / L"test_1000lines.txt";

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// OLEを初期化する
		if (FAILED(::OleInitialize(nullptr)))
			FAIL();

		// UI Automationを初期化する
		SetUpUia();

		// テスト用ファイル作成
		std::wofstream fs(gm_TestDataPath);
		for (int n = 1; n <= 1000; n++) {
			fs << n << std::endl;
		}
		fs.close();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		std::error_code ec;

		// テスト用ファイルの後始末
		if (fexist(gm_TestDataPath)) {
			std::filesystem::remove(gm_TestDataPath, ec);
		}

		if (const auto pluginPath = GetIniFileName().remove_filename().append(L"plugins"); fexist(pluginPath)) {
			std::filesystem::remove_all(pluginPath, ec);
		}

		// UI Automationをシャットダウンする
		TearDownUia();

		// OLEをシャットダウンする
		::OleUninitialize();
	}

	/*!
	 * 設定ファイルのパス
	 *
	 * GetIniFileNameを使ってtests1.iniのパスを取得する。
	 */
	std::filesystem::path iniPath;

	virtual ~TWinMainTest() = default;

	virtual std::wstring_view GetProfileName() const = 0;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テスト用プロファイル名
		const std::wstring_view profileName{ GetProfileName() };

		// コマンドラインのインスタンスを用意する
		CCommandLine commandLine;
		const auto strCommandLine = std::format(LR"(-PROF="{}")", profileName);
		commandLine.ParseCommandLine(strCommandLine.c_str(), false);

		// プロセスのインスタンスを用意する
		CControlProcess dummy(nullptr, strCommandLine.data());

		// INIファイルのパスを取得
		iniPath = GetIniFileName();

		// INIファイルを削除する
		if (fexist(iniPath)) {
			std::error_code ec;
			std::filesystem::remove(iniPath, ec);
		}

		// テスト用INIファイル作成
		// Grepダイアログを日本語で表示させるために設定を入れる
		constexpr std::array iniLines = {
			// 全般設定を出力
			u8"[Common]"sv,
			u8"szLanguageDll="sv,			// 言語DLLの指定(空にすると日本語になる)
			u8"bTaskTrayUse=1"sv,			// タスクトレイのアイコンを使う
			u8"bTaskTrayStay=1"sv,			// タスクトレイのアイコンを常駐
		};
		cxx::writeTextFile(iniPath, iniLines);
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// INIファイルを削除する
		if (fexist(iniPath)) {
			std::error_code ec;
			std::filesystem::remove(iniPath, ec);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (const std::wstring_view profileName{ GetProfileName() }; !profileName.empty()) {
			std::filesystem::remove_all(iniPath.parent_path());
		}
	}

	/*!
	 * wWinMain呼出ラッパー
	 *
	 * テスト内で使うためのラッパー。
	 * 関数が呼出元に返らないことをマークしたバージョン。
	 */
	static NORETURN void StartEditorProcess(const std::wstring& command) {
		exit(testing::StartEditorProcess(command));
	}

	/*!
	 * @brief コントロールプロセスを起動し、終了指示を出して、終了を待つ
	 */
	void CControlProcess_StartAndTerminate(std::wstring_view profileName) const
	{
		// コントロールプロセスを起動する
		auto cp = testing::CreateControlProcess(profileName);
		EXPECT_THAT(cp, NotNull());

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, cp.dwProcessId);
	}
};

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
struct WinMainTest : public TWinMainTest<::testing::TestWithParam<std::wstring_view>> {
	std::wstring_view GetProfileName() const override {
		return GetParam();
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
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// 設定ファイルの出力を確認したいので、テストスイートの設定ファイルを削除する
	std::filesystem::remove(iniPath);

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(profileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_THAT(fexist(iniPath), IsTrue());

	// コントロールプロセスを起動し、終了指示を出して、終了を待つ
	CControlProcess_StartAndTerminate(profileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_THAT(fexist(iniPath), IsTrue());
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  エディタプロセスを実行する。
 */
TEST_P(WinMainTest, runEditorProcess)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// プラグイン設定フォルダー
	const auto pluginPath = GetIniFileName().remove_filename().append(L"plugins");

	// プラグイン定義を展開する
	extract_zip_resource(IDR_ZIPRES1, pluginPath);
	extract_zip_resource(IDR_ZIPRES4, pluginPath);

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::error_code ec;
	std::filesystem::remove(iniPath, ec);

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)

		// ツールバー設定を出力
		u8"[Toolbar]"sv,
		u8"bToolBarIsFlat=1"sv,
		u8"nTBB[000]=1"sv,
		u8"nTBB[001]=25"sv,
		u8"nTBB[002]=3"sv,
		u8"nTBB[003]=4"sv,
		u8"nTBB[004]=0"sv,
		u8"nTBB[005]=33"sv,
		u8"nTBB[006]=34"sv,
		u8"nTBB[007]=0"sv,
		u8"nTBB[008]=87"sv,
		u8"nTBB[009]=88"sv,
		u8"nTBB[010]=0"sv,
		u8"nTBB[011]=225"sv,
		u8"nTBB[012]=226"sv,
		u8"nTBB[013]=227"sv,
		u8"nTBB[014]=228"sv,
		u8"nTBB[015]=229"sv,
		u8"nTBB[016]=230"sv,
		u8"nTBB[017]=0"sv,
		u8"nTBB[018]=232"sv,
		u8"nTBB[019]=0"sv,
		u8"nTBB[020]=264"sv,
		u8"nTBB[021]=265"sv,
		u8"nTBB[022]=266"sv,
		u8"nTBB[023]=0"sv,
		u8"nTBB[024]=346"sv,
		u8"nTBB[025]=246"sv,
		u8"nTBB[026]=384"sv,
		u8"nTBB[027]=246"sv,
		u8"nTBB[028]=384"sv,
		u8"nToolBarButtonNum=29"sv,
		u8""sv,

		// プラグイン設定を出力
		u8"[Plugin]"sv,
		u8"EnablePlugin=1"sv,
		u8"P[00].CmdNum=1"sv,
		u8"P[00].Id=TestWshPlugin"sv,
		u8"P[00].Name=test-plugin"sv,
		u8"P[01].CmdNum=2"sv,
		u8"P[01].Id=TestDllPlugin"sv,
		u8"P[01].Name=test-dllplugin"sv,

		// プリンター設定を出力
		u8"[Print]"sv,
		u8"PS[00].bColorPrint=0"sv,
		u8"PS[00].bKinsokuHead=0"sv,
		u8"PS[00].bKinsokuKuto=0"sv,
		u8"PS[00].bKinsokuRet=0"sv,
		u8"PS[00].bKinsokuTail=0"sv,
		u8"PS[00].lfFooter=0,0,0,0,0,0,0,0,0,0,0,0,0"sv,
		u8"PS[00].lfFooterFaceName="sv,
		u8"PS[00].lfHeader=0,0,0,0,0,0,0,0,0,0,0,0,0"sv,
		u8"PS[00].lfHeaderFaceName="sv,
		u8"PS[00].nFooterPointSize=0"sv,
		u8"PS[00].nHeaderPointSize=0"sv,
		u8"PS[00].nInts=12,24,1,70,30,100,200,200,100,1,9,1,0,1,0,0,1,0,0"sv,
		u8"PS[00].szDevice=Microsoft Print to PDF"sv,
		u8"PS[00].szDriver=winspool"sv,
		u8"PS[00].szFF=ＭＳ 明朝"sv,
		u8"PS[00].szFFZ=ＭＳ 明朝"sv,
		u8"PS[00].szFTF[0]="sv,
		u8"PS[00].szFTF[1]=- $p / $P -"sv, // ページ番号 / 総ページ数
		u8"PS[00].szFTF[2]="sv,
		u8"PS[00].szHF[0]=$f"sv,
		u8"PS[00].szHF[1]=$Q"sv,
		u8"PS[00].szHF[2]=$d $t"sv,
		u8"PS[00].szOutput=PORTPROMPT:"sv,
		u8"PS[00].szSName=印刷設定 1"sv,
	};
	cxx::writeTextFile(iniPath, iniLines);

	// プロファイル指定がある場合の追加テスト
	if (!profileName.empty()) {
		// ファイル出力ストリームをバイナリモードで開く
		std::ofstream fs(iniPath, std::ios::binary | std::ios::app);

		// ダークモードをONにする
		fs << "[Common]\r\n";
		fs << "bDarkMode=1\r\n";

		fs.close();
	}

	// コントロールプロセスを起動する
	auto cp = testing::CreateControlProcess(profileName);
	EXPECT_THAT(cp, NotNull());

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

		L"PrintPreview();"sv,			// 印刷プレビュー出す
		L"WinMaximize();"sv,
		L"WinRestore();"sv,
		L"WinMinimize();"sv,
		L"WinRestore();"sv,
		L"PrintPreview();"sv,			// 印刷プレビュー消す

		L"SplitWinVH();"sv,
		L"NextWindow();NextWindow();NextWindow();NextWindow();"sv,

		L"WheelDown();"sv,
		L"WheelUp();"sv,
		L"WheelRight();"sv,
		L"WheelLeft();"sv,

		L"WinMaximize();"sv,
		L"WinRestore();"sv,
		L"WinMinimize();"sv,
		L"WinRestore();"sv,

		L"PrevWindow();PrevWindow();PrevWindow();PrevWindow();"sv,
		L"SplitWinVH();"sv,

		L"SplitWinV();"sv,
		L"SplitWinH();"sv,
		L"SplitWinH();"sv,
		L"SplitWinV();"sv,

		L"SplitWinH();"sv,
		L"SplitWinV();"sv,
		L"SplitWinV();"sv,
		L"SplitWinH();"sv,

		L"Outline(1);"sv,				//アウトライン解析をリロード
		L"Outline(2);"sv,				//アウトライン解析を閉じる

		L"ShowFunckey();"sv,			//ShowFunckey 消す
		L"ShowMiniMap();"sv,			//ShowMiniMap 消す
		L"ShowTab();"sv,				//ShowTab 消す

		L"ShowToolbar();"sv,			//ShowToolbar 消す
		L"ShowStatusbar();"sv,			//ShowStatusbar 消す
		L"ShowStatusbar();"sv,			//ShowStatusbar 出す
		L"ShowToolbar();"sv,			//ShowToolbar 出す

		L"ExpandParameter('$I');"sv,	// INIファイルパスの取得(呼ぶだけ)

		L"ChgmodINS();"sv,
		L"ChgmodINS();"sv,

		L"GoFileTop();"sv,
		L"SearchNext('3');"sv,			// 検索(呼ぶだけ)
		L"GoFileEnd();"sv,
		L"SearchPrev('3');"sv,			// 検索(呼ぶだけ)

		L"GoFileTop();"sv,
		// ↓コマンドライン経由なので日本語入れると危険！
		L"Replace('3', 'threeeee');"sv,	// 置換(呼ぶだけ)
		L"Undo();"sv,
		L"Redo();"sv,
		L"Undo();"sv,

		// ↓コマンドライン経由なので日本語入れると危険！
		L"ReplaceAll('3', 'threeee');"sv,	// すべて置換(呼ぶだけ)
		L"Undo();"sv,

		// OLEクリップボード
		L"SetClipboard(3, 'test');"sv,
		L"GetClipboard();"sv,
		L"ClipboardEmpty();"sv,

		// 生クリップボード
		L"SetClipboardByFormat('test', '12345', 0, -1);"sv,
		L"IsIncludeClipboardFormat('12345');"sv,
		L"GetClipboardByFormat('12345', 0, 0);"sv,
		L"ClipboardEmpty();"sv,

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

		L"ExitAll();"sv		//NOTE: このコマンドにより、エディタプロセスは起動された直後に終了する。
	};

	// 起動時実行マクロを組み立てる
	const auto strStartupMacro = std::accumulate(macroCommands.cbegin(), macroCommands.cend(), std::wstring(), [](const std::wstring& a, std::wstring_view b) { return a + std::data(b); });

	// コマンドラインを組み立てる
	std::wstring command(gm_TestDataPath);
	command += std::format(LR"( -PROF="{}")", profileName);
	command += std::format(LR"( -MTYPE=js -M="{}")", std::regex_replace(strStartupMacro, std::wregex(LR"(")"), LR"("")"));

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	EXPECT_EXIT({ StartEditorProcess(command); }, ::testing::ExitedWithCode(0), ".*" );

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, cp.dwProcessId);

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_THAT(fexist(iniPath), IsTrue());
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
struct WinMainFuncTest : public TWinMainTest<::testing::Test> {
	std::wstring_view GetProfileName() const override {
		return L"";
	}
};

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepを実行する。
 */
TEST_F(WinMainFuncTest, DoGrep001)
{
	RunGuiTest([this] {
		// テスト用プロファイル名
		const auto profileName{ GetProfileName() };

		// コントロールプロセスを起動する
		auto cp = testing::CreateControlProcess(profileName);
		EXPECT_THAT(cp, NotNull());

		std::array args{
			LR"(-GREPMODE)"s,
			LR"(-GKEY="localhost")"s,
			LR"(-GFILE="*.*;#en-US;#DriverData;#UMDF;#udc;#mde;#wd;!*.sys;!*.dll;!*.exe;!*.mui;!*.nls;!*.chm;!*.dat;!*.tmp;!*.wdf")"s,
			LR"(-GFOLDER="C:\WINDOWS\System32\Drivers")"s,
			LR"(-GOPT=SP1)"s
		};

		// エディタープロセスを起動する
		auto ep = testing::CreateEditorProcess(args, profileName);
		EXPECT_THAT(ep, NotNull());
		EXPECT_THAT(ep.hWnd, NotNull());

		// 編集ウインドウにクローズを要求する
		testing::RequestForeignWindowClose(ep.hWnd);

		// エディタープロセスが終了するのを待つ
		ep.lock();

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, cp.dwProcessId);
	});
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  アウトプットウインドウを表示する。
 */
TEST_F(WinMainFuncTest, OpenDebugWindow001)
{
	RunGuiTest([this] {
		// テスト用プロファイル名
		const auto profileName{ GetProfileName() };

		// コントロールプロセスを起動する
		auto cp = testing::CreateControlProcess(profileName);
		EXPECT_THAT(cp, NotNull());

		// エディタープロセスを起動する
		auto ep = testing::CreateEditorProcess(std::array{ LR"(-DEBUGMODE)" }, profileName);
		EXPECT_THAT(ep, NotNull());
		EXPECT_THAT(ep.hWnd, NotNull());

		// 編集ウインドウにクローズを要求する
		testing::RequestForeignWindowClose(ep.hWnd);

		// エディタープロセスが終了するのを待つ
		ep.lock();

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, cp.dwProcessId);
	});
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepダイアログを表示してキャンセルで閉じる。
 */
TEST_F(WinMainFuncTest, ShowDlgGrep101)
{
	RunGuiTest([this] {
		// テスト用プロファイル名
		const auto profileName{ GetProfileName() };

		// コントロールプロセスを起動する
		auto cp = testing::CreateControlProcess(profileName);
		EXPECT_THAT(cp, NotNull());

		// エディタープロセスを起動する
		auto ep = testing::CreateSakuraProcess(std::array{ LR"(-GREPDLG)", LR"(-GREPMODE)" }, profileName);
		EXPECT_THAT(ep, NotNull());

		// Grepダイアログが表示されるのを待って閉じる
		const auto hDlgGrep = WaitForDialog(L"Grep", 30000);
		EXPECT_THAT(hDlgGrep, NotNull());

		// Grepダイアログを閉じる
		EmulateInvokeButton(hDlgGrep, L"キャンセル(X)");

		// 編集ウインドウが有効になるのを待つ
		const auto hWndFound = WaitForEditor();
		EXPECT_THAT(hWndFound, NotNull());

		// 編集ウインドウにクローズを要求する
		testing::RequestForeignWindowClose(hWndFound);

		// エディタープロセスが終了するのを待つ
		ep.lock();

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, cp.dwProcessId);
	});
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  プロファイルマネージャを表示してキャンセルで閉じる。
 */
TEST_F(WinMainFuncTest, ShowDlgProfileMgr101)
{
	RunGuiTest([this] {
		// テスト用プロファイル名
		const auto profileName{ GetProfileName() };

		// エディタープロセスを起動する
		auto ep = testing::CreateSakuraProcess(std::array{ LR"(-PROFMGR)" }, profileName);
		EXPECT_THAT(ep, NotNull());

		// プロファイルマネージャが表示されるのを待って閉じる
		const auto hWndDlgProfileMgr = WaitForDialog(L"プロファイルマネージャ");
		EXPECT_THAT(hWndDlgProfileMgr, NotNull());

		// プロファイルマネージャを閉じる
		EmulateInvokeButton(hWndDlgProfileMgr, L"閉じる(X)");

		// エディタープロセスが終了するのを待つ
		ep.lock();
	});
}

} // namespace winmain
