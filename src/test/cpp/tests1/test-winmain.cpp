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
#include "util/file.h"
#include "config/system_constants.h"
#include "_main/CProcessFactory.h"
#include "_main/CControlProcess.h"

#include "testing/HResultEq.hpp"
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

} // namespace cxx

namespace testing {

/*!
 * @brief コントロールプロセスを起動する
 *
 * @param profileName プロファイル名
 * @return コントロールプロセスのプロセスID
 * @note 使い物になるかどうか試作してみた
 */
DWORD CreateControlProcess(std::wstring_view profileName)
{
	return CProcess::CreateControlProcess(profileName);
}

/*!
 * @brief エディタープロセスを起動する
 *
 * @tparam T コマンドライン引数のコンテナ型
 * @param args コマンドライン引数
 * @param profileName プロファイル名
 * @return 起動したプロセスのハンドルオブジェクト
 */
template<class T>
	requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
cxx::ProcessHolder CreateEditorProcess(
	const std::optional<std::filesystem::path>& optFilePath,
	const T& args,
	std::wstring_view profileName
)
{
	// コマンドライン引数の編集用vector
	std::vector<std::wstring> commandArgs{ std::begin(args), std::end(args) };

	// コマンドラインに -CODE 指定がない場合は付与する
	if (const auto found = std::ranges::find_if(args, [](const std::wstring& arg) { return std::regex_match(arg, std::wregex(LR"(\s*-CODE.*)", std::wregex::icase)); }); found == args.end()) {
		commandArgs.emplace_back(std::format(LR"(-CODE={})", static_cast<int>(CODE_AUTODETECT)));
	}

	return CProcess::CreateEditorProcess(optFilePath, commandArgs, std::optional<std::wstring>(profileName));
}

/*!
 * @brief エディタープロセスを起動する
 *
 * @tparam T コマンドライン引数のコンテナ型
 * @param args コマンドライン引数
 * @param profileName プロファイル名
 * @return 起動したプロセスのハンドルオブジェクト
 */
template<class T>
	requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
cxx::ProcessHolder CreateEditorProcess(
	const T& args,
	std::wstring_view profileName
)
{
	return CreateEditorProcess(std::nullopt, args, profileName);
}

//! 外部ウインドウにクローズを要求する
void RequestForeignWindowClose(HWND hWnd)
{
	// ウインドウが閉じられるまで繰り返す
	while (::IsWindow(hWnd)) {
		// ウインドウにクローズを要求する
		if (!::SendMessageTimeoutW(hWnd, WM_CLOSE, 0, 0,
			SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT,
			5000,
			nullptr
		)) {
			// Sendが失敗したらPostしておく
			::PostMessageW(hWnd, WM_CLOSE, 0, 0);

			// 少し待つ
			::Sleep(100);
		}
	}
}

//! 外部プロセスの終了を待つ
void WaitForForeignProcessExit(const cxx::HandleHolder& process)
{
	// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	if (!process.try_lock_for(std::chrono::milliseconds(45000))) {
		// 終了できないなら強制終了させる
		if (const auto exitCode = 1; !::TerminateProcess(process.get(), exitCode)) {
			cxx::raise_system_error("waitProcess is timeout and terminate process failed.");
		}

		// TerminateProcess は非同期なので操作完了を待つ
		if (!process.try_lock_for(std::chrono::milliseconds(5000))) {
			cxx::raise_system_error("waitProcess is timeout and force terminate is timeout.");
		}
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
	cxx::HandleHolder process = ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, dwControlProcessId);
	if (!process) {
		// プロセスIDが無効は「既に終了している」なので、除外する
		if (ERROR_INVALID_PARAMETER == ::GetLastError()) {
			return;
		}
		cxx::raise_system_error("hControlProcess can't be opened.");
	}

	// メインウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	WaitForForeignProcessExit(process);
}

} // namespace testing

namespace winmain {

/*!
 * WinMain起動テストのためのフィクスチャクラス
 *
 * 設定ファイルを使うテストは「設定ファイルがない状態」からの始動を想定しているので
 * 始動前に設定ファイルを削除するようにしている。
 * テスト実行後に設定ファイルを残しておく意味はないので終了後も削除している。
 */
struct WinMainTest : public ::testing::TestWithParam<std::wstring_view>, public window::UiaTestSuite {
	/*!
	 * テスト用ファイル1のパス
	 */
	static inline std::filesystem::path gm_TestDataPath1 = std::filesystem::current_path() / L"test_1000lines.txt";

	/*!
	 * テスト用ファイル2のパス
	 */
	static inline std::filesystem::path gm_TestDataPath2 = std::filesystem::current_path() / L"test_2000lines.txt";

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
		std::wofstream fos1(gm_TestDataPath1);
		for (int n = 1; n <= 1000; n++) {
			fos1 << n << std::endl;
		}
		fos1.close();

		std::wofstream fos2(gm_TestDataPath2);
		for (int n = 1; n <= 2000; n++) {
			fos2 << n << std::endl;
		}
		fos2.close();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// テスト用ファイルの後始末
		if (fexist(gm_TestDataPath1)) {
			std::filesystem::remove(gm_TestDataPath1);
		}
		if (fexist(gm_TestDataPath2)) {
			std::filesystem::remove(gm_TestDataPath2);
		}

		if (const auto pluginPath = GetIniFileName().remove_filename().append(L"plugins"); fexist(pluginPath)) {
			std::error_code ec;
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

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テスト用プロファイル名
		const std::wstring_view profileName(GetParam());

		// プロセスのインスタンスを用意する
		const auto dummy = CProcessFactory().CreateInstance(std::format(LR"(-NOWIN -PROF="{}")", profileName));

		// INIファイルのパスを取得
		iniPath = GetIniFileName();

		// INIファイルを削除する
		if (fexist(iniPath)) {
			std::filesystem::remove(iniPath);
		}

		// テスト用INIファイル作成
		// Grepダイアログを日本語で表示させるために設定を入れる
		constexpr std::array iniLines = {
			// 全般設定を出力
			u8"[Common]"sv,
			u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
		};
		cxx::writeTextFile(iniPath, iniLines);
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// INIファイルを削除する
		if (fexist(iniPath)) {
			std::filesystem::remove(iniPath);
		}

		// プロファイル指定がある場合、フォルダーも削除しておく
		if (const std::wstring_view profileName(GetParam()); !profileName.empty()) {
			std::error_code ec;
			std::filesystem::remove_all(iniPath.parent_path(), ec);
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
		const auto dwControlProcessId = testing::CreateControlProcess(profileName);

		// コントロールプロセスに終了指示を出して終了を待つ
		testing::TerminateControlProcess(profileName, dwControlProcessId);
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
	std::filesystem::remove(iniPath);

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
	std::wstring command(gm_TestDataPath1);
	command += std::format(LR"( -PROF="{}")", profileName);
	command += std::format(LR"( -MTYPE=js -M="{}")", std::regex_replace(strStartupMacro, std::wregex(LR"(")"), LR"("")"));

	// テストプログラム内のグローバル変数を汚さないために、別プロセスで起動させる
	EXPECT_EXIT({ StartEditorProcess(command); }, ::testing::ExitedWithCode(0), ".*" );

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName);

	// コントロールプロセスが終了すると、INIファイルが作成される
	EXPECT_THAT(fexist(iniPath), IsTrue());
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  タブバー有効で、別グループにしない場合、既存タブグループにフィットさせる。
 */
TEST_P(WinMainTest, _CalcInitialRect001)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::filesystem::remove(iniPath);

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
		u8"bDarkMode=1"sv,		// ダークモードをONにする
		u8"bDispTabWnd=1"sv,	// タブバーを表示する
	};
	cxx::writeTextFile(iniPath, iniLines);

	// 1つ目のエディタープロセスを起動する
	const auto ep1 = testing::CreateEditorProcess(gm_TestDataPath1, std::array{ LR"(-Y=3)"s }, profileName);

	// 編集ウインドウが有効になるのを待つ
	WaitForEditor();

	// 2つ目のエディタープロセスを起動する
	const auto ep2 = testing::CreateEditorProcess(gm_TestDataPath2, std::array{ LR"(-Y=3)"s }, profileName);

	WaitForThread(ep2.dwThreadId);

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  タブバー有効で、別グループにしない場合、既存タブグループにフィットさせる。
 */
TEST_P(WinMainTest, _CalcInitialRect002)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::filesystem::remove(iniPath);

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
		u8"bDarkMode=1"sv,		// ダークモードをONにする
		u8"bDispTabWnd=1"sv,	// タブバーを表示する
	};
	cxx::writeTextFile(iniPath, iniLines);

	// 1つ目のエディタープロセスを起動する
	const auto ep1 = testing::CreateEditorProcess(gm_TestDataPath1, std::array{ LR"(-Y=3)"s }, profileName);

	// 編集ウインドウが有効になるのを待つ
	const auto hWndFound = WaitForEditor();

	::ShowWindow(hWndFound, SW_MINIMIZE);

	// 2つ目のエディタープロセスを起動する
	const auto ep2 = testing::CreateEditorProcess(gm_TestDataPath2, std::array{ LR"(-Y=3)"s }, profileName);

	WaitForThread(ep2.dwThreadId);

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  既に開いているウインドウをアクティブにする。
 */
TEST_P(WinMainTest, ActivateOpenedWindow001)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::filesystem::remove(iniPath);

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
		u8"bDarkMode=1"sv,		// ダークモードをONにする
	};
	cxx::writeTextFile(iniPath, iniLines);

	// 1つ目のエディタープロセスを起動する
	const auto ep1 = testing::CreateEditorProcess(gm_TestDataPath1, std::array{ LR"(-Y=3)"s }, profileName);

	// 編集ウインドウが有効になるのを待つ
	const auto hWndFound = WaitForEditor();

	// 2つ目のエディタープロセスを起動する
	const auto ep2 = testing::CreateEditorProcess(gm_TestDataPath1, std::array{ LR"(-Y=3)"s }, profileName);

	// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	testing::WaitForForeignProcessExit(ep2);

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  トレイアイコンクリックメニューからGrepを実行する。
 */
TEST_P(WinMainTest, DoGrep001)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::filesystem::remove(iniPath);

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
		u8"bDarkMode=1"sv,		// ダークモードをONにする

		// 検索キーを出力
		u8"[Keys]"sv,
		u8"_SEARCHKEY_Counts=1"sv,
		u8"SEARCHKEY[00]=localhost"sv,

		// Grep設定を出力
		u8"[Grep]"sv,
		u8"_GREPFILE_Counts=1"sv,
		u8"GREPFILE[00]=*.*"sv,
		u8"_GREPFOLDER_Counts=1"sv,
		u8"GREPFOLDER[00]=C:\\WINDOWS\\System32\\Drivers\\"sv,
		u8"_GREPEXCLUDEFOLDER_Counts=1"sv,
		u8"GREPEXCLUDEFOLDER[00]=.git;en-US"sv,
		u8"_GREPEXCLUDEFILE_Counts=1"sv,
		u8"GREPEXCLUDEFILE[00]=*.msi;*.exe;*.dll;*.obj;*.pdb;*.chm;*.nls;*.dat;*.sys;*.tmp"sv,
	};
	cxx::writeTextFile(iniPath, iniLines);

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);
	EXPECT_THAT(dwControlProcessId, Ne(0));

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName);
	EXPECT_THAT(hTrayWnd, NotNull());

	const std::jthread t([hTrayWnd] {
		cxx::com_pointer<ITrayWnd> pDispatch = nullptr;
		EXPECT_HRESULT_SUCCEEDED(::AccessibleObjectFromWindow(
			hTrayWnd,
			OBJID_NATIVEOM,
			IID_PPV_ARGS(&pDispatch)
		));

		// トレイアイコン左クリックメニューを表示させる
		DISPID dispid = 1; // ShowTrayClickMenu
		DISPPARAMS params = {};
		EXPECT_HRESULT_SUCCEEDED(pDispatch->Invoke(
			dispid,
			IID_NULL,
			LOCALE_USER_DEFAULT,
			DISPATCH_METHOD,
			&params,
			nullptr,
			nullptr,
			nullptr
		));
	});

	EmulateSelectPopupMenu(L"Grep(G)...");

	// Grepダイアログが表示されるのを待って実行する
	if(const auto hDlgGrep = WaitForDialog(L"Grep")) {
		EmulateHitEnter();
	}

	// Grepダイアログが閉じられるのを待つ
	bool dlgClosed = false;
	for (const auto startTick = ::GetTickCount64(); ::GetTickCount64() - startTick < 5000;) {
		if (const auto hWndFound = ::FindWindowW(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), L"Grep"); !hWndFound) {
			dlgClosed = true;
			break;
		}
		Sleep(10);  // 10msスリープしてリトライ
	}

	EXPECT_TRUE(dlgClosed) << "Grep dialog should be closed.";

	// 編集ウインドウが有効になるのを待つ
	const auto hWndFound = WaitForEditor();

	// 編集ウインドウからプロセスIDを取得する
	DWORD dwEditorProcessId;
	::GetWindowThreadProcessId(hWndFound, &dwEditorProcessId);
	if (!dwControlProcessId) {
		cxx::raise_system_error("dwEditorProcessId can't be retrived.");
	}

	cxx::HandleHolder ep = ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, dwEditorProcessId);

	// 編集ウインドウを閉じる
	testing::RequestForeignWindowClose(hWndFound);

	// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	testing::WaitForForeignProcessExit(ep);

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  コマンドラインからGrepダイアログを表示してGrepを実行する。
 */
TEST_P(WinMainTest, DoGrep002)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// ケース独自の設定ファイルを使うので、一旦削除する
	std::filesystem::remove(iniPath);

	// テスト用INIファイル作成
	// 標準機能をできるだけ動かすために設定を入れる
	constexpr std::array iniLines = {
		// 全般設定を出力
		u8"[Common]"sv,
		u8"szLanguageDll="sv,	// 言語DLLの指定(空にすると日本語になる)
		u8"bDarkMode=1"sv,		// ダークモードをONにする
	};
	cxx::writeTextFile(iniPath, iniLines);

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);
	EXPECT_THAT(dwControlProcessId, Ne(0));

	std::array args{
		LR"(-GREPDLG)"s,
		LR"(-GKEY="localhost")"s,
		LR"(-GREPR="local")"s,
		LR"(-GFILE="*.*;#en-US;!*.msi;!*.exe;!*.dll;!*.obj;!*.pdb;!*.chm;!*.nls;!*.dat;!*.sys;!*.tmp")"s,
		LR"(-GFOLDER="C:\WINDOWS\System32\Drivers")"s,
		LR"(-GOPT=SP1)"s
	};

	// エディタープロセスを起動する
	const auto ep = testing::CreateEditorProcess(args, profileName);

	// Grepダイアログが表示されるのを待って実行する
	if(const auto hDlgGrep = WaitForDialog(L"Grep")) {
		EmulateHitEnter();
	}

	// Grepダイアログが閉じられるのを待つ
	bool dlgClosed = false;
	for (const auto startTick = ::GetTickCount64(); ::GetTickCount64() - startTick < 5000;) {
		if (const auto hWndFound = ::FindWindowW(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), L"Grep"); !hWndFound) {
			dlgClosed = true;
			break;
		}
		Sleep(10);  // 10msスリープしてリトライ
	}

	EXPECT_TRUE(dlgClosed) << "Grep dialog should be closed.";

	// 編集ウインドウが有効になるのを待つ
	const auto hWndFound = WaitForEditor();

	// 編集ウインドウを閉じる
	testing::RequestForeignWindowClose(hWndFound);

	// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	testing::WaitForForeignProcessExit(ep);

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  アウトプットウインドウを表示する。
 */
TEST_P(WinMainTest, OpenDebugWindow001)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// エディタープロセスを起動する
	const auto ep = testing::CreateEditorProcess(std::array{ LR"(-DEBUGMODE)" }, profileName);

	// 編集ウインドウが有効になるのを待って閉じる
	WaitForEditor();

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  2つ目のアウトプットウインドウを表示しようとしてみる。
 */
TEST_P(WinMainTest, OpenDebugWindow002)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// エディタープロセスを起動する
	const auto ep1 = testing::CreateEditorProcess(std::array{ LR"(-DEBUGMODE)" }, profileName);

	// アウトプットウインドウが有効になるのを待つ
	WaitForEditor();

	// 2つ目のエディタープロセスを起動する
	const auto ep2 = testing::CreateEditorProcess(std::array{ LR"(-DEBUGMODE)" }, profileName);

	WaitForThread(ep2.dwThreadId);

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepを実行する。
 */
TEST_P(WinMainTest, OpenFile001)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);
	EXPECT_THAT(dwControlProcessId, Ne(0));

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName);
	EXPECT_THAT(hTrayWnd, NotNull());

	const std::jthread t([hTrayWnd] {
		cxx::com_pointer<ITrayWnd> pDispatch = nullptr;
		EXPECT_HRESULT_SUCCEEDED(::AccessibleObjectFromWindow(
			hTrayWnd,
			OBJID_NATIVEOM,
			IID_PPV_ARGS(&pDispatch)
		));

		// トレイアイコン左クリックメニューを表示させる
		DISPID dispid = 1; // ShowTrayClickMenu
		DISPPARAMS params = {};
		EXPECT_HRESULT_SUCCEEDED(pDispatch->Invoke(
			dispid,
			IID_NULL,
			LOCALE_USER_DEFAULT,
			DISPATCH_METHOD,
			&params,
			nullptr,
			nullptr,
			nullptr
		));
	});

	EmulateSelectPopupMenu(L"開く(O)...");

	if (const auto hDlgOpenFile = WaitForDialog(L"開く")) {
		EmulateSetValue(GetFocusedElement(), gm_TestDataPath1.c_str());
		EmulateHitEnter();
	}

	// 編集ウインドウが有効になるのを待つ
	WaitForEditor();

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepを実行する。
 */
TEST_P(WinMainTest, OpenNewEditor001)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);
	EXPECT_THAT(dwControlProcessId, Ne(0));

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName);
	EXPECT_THAT(hTrayWnd, NotNull());

	const std::jthread t([hTrayWnd] {
		cxx::com_pointer<ITrayWnd> pDispatch = nullptr;
		EXPECT_HRESULT_SUCCEEDED(::AccessibleObjectFromWindow(
			hTrayWnd,
			OBJID_NATIVEOM,
			IID_PPV_ARGS(&pDispatch)
		));

		// トレイアイコン左クリックメニューを表示させる
		DISPID dispid = 1; // ShowTrayClickMenu
		DISPPARAMS params = {};
		EXPECT_HRESULT_SUCCEEDED(pDispatch->Invoke(
			dispid,
			IID_NULL,
			LOCALE_USER_DEFAULT,
			DISPATCH_METHOD,
			&params,
			nullptr,
			nullptr,
			nullptr
		));
	});

	EmulateSelectPopupMenu(L"新規作成(N)");

	// 編集ウインドウが有効になるのを待つ
	WaitForEditor();

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepを実行する。
 */
TEST_P(WinMainTest, OpenNewEditor002)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);
	EXPECT_THAT(dwControlProcessId, Ne(0));

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName);
	EXPECT_THAT(hTrayWnd, NotNull());

	const std::jthread t([hTrayWnd] {
		cxx::com_pointer<ITrayWnd> pDispatch = nullptr;
		EXPECT_HRESULT_SUCCEEDED(::AccessibleObjectFromWindow(
			hTrayWnd,
			OBJID_NATIVEOM,
			IID_PPV_ARGS(&pDispatch)
		));

		// トレイアイコンダブルクリックする
		DISPID dispid = 3; // OpenNewEditor
		DISPPARAMS params = {};
		EXPECT_HRESULT_SUCCEEDED(pDispatch->Invoke(
			dispid,
			IID_NULL,
			LOCALE_USER_DEFAULT,
			DISPATCH_METHOD,
			&params,
			nullptr,
			nullptr,
			nullptr
		));
	});

	// 編集ウインドウが有効になるのを待つ
	WaitForEditor();

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepダイアログを表示してキャンセルで閉じる。
 */
TEST_P(WinMainTest, ShowDlgGrep101)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);

	// エディタープロセスを起動する
	const auto ep = testing::CreateEditorProcess(std::array{ LR"(-GREPDLG)", LR"(-GREPMODE)" }, profileName);

	// Grepダイアログが表示されるのを待って閉じる
	const auto hDlgGrep = WaitForDialog(L"Grep", 30000);
	EmulateInvokeButton(hDlgGrep, L"キャンセル(X)");

	bool dlgClosed = false;
	for (const auto startTick = ::GetTickCount64(); ::GetTickCount64() - startTick < 5000;) {
		if (const auto hWndFound = ::FindWindowW(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), L"Grep"); !hWndFound) {
			dlgClosed = true;
			break;
		}
		Sleep(10);  // 10msスリープしてリトライ
	}

	EXPECT_TRUE(dlgClosed) << "Grep dialog should be closed.";

	// 編集ウインドウを閉じる
	WaitForEditor();

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grep置換ダイアログを表示してキャンセルで閉じる。
 */
TEST_P(WinMainTest, ShowDlgGrep102)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);

	std::array args{
		LR"(-GREPDLG)"s,
		LR"(-GKEY="localhost")"s,
		LR"(-GREPR="local")"s,
		LR"(-GFILE="*.*;#en-US;!*.msi;!*.exe;!*.dll;!*.obj;!*.pdb;!*.chm;!*.nls;!*.dat;!*.sys;!*.tmp")"s,
		LR"(-GFOLDER="C:\WINDOWS\System32\Drivers")"s,
		LR"(-GOPT=SP1)"s
	};

	// エディタープロセスを起動する
	const auto ep = testing::CreateEditorProcess(args, profileName);

	// Grepダイアログが表示されるのを待って閉じる
	const auto hDlgGrep = WaitForDialog(L"Grep", 30000);
	EmulateInvokeButton(hDlgGrep, L"キャンセル(X)");

	bool dlgClosed = false;
	for (const auto startTick = ::GetTickCount64(); ::GetTickCount64() - startTick < 5000;) {
		if (const auto hWndFound = ::FindWindowW(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), L"Grep"); !hWndFound) {
			dlgClosed = true;
			break;
		}
		Sleep(10);  // 10msスリープしてリトライ
	}

	EXPECT_TRUE(dlgClosed) << "Grep dialog should be closed.";

	// 編集ウインドウを閉じる
	WaitForEditor();

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  プロファイルマネージャを表示してキャンセルで閉じる。
 */
TEST_P(WinMainTest, ShowDlgProfileMgr101)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// エディタープロセスを起動する
	const auto ep = testing::CreateEditorProcess(std::array{ LR"(-PROFMGR)" }, profileName);

	// プロファイルマネージャが表示されるのを待って閉じる
	const auto hWndDlgProfileMgr = WaitForWindow(MAKEINTRESOURCEW(dialog::ModalDialogCloser::DIALOG_CLASS), L"プロファイルマネージャ");
	EmulateInvokeButton(hWndDlgProfileMgr, L"閉じる(X)");

	// 編集ウインドウが閉じられた後、プロセスが完全に終了するまで待つ
	testing::WaitForForeignProcessExit(ep);
}

/*!
 * @brief WinMainを起動してみるテスト
 *  プログラムが起動する正常ルートに潜む障害を検出するためのもの。
 *  Grepを実行する。
 */
TEST_P(WinMainTest, ShowPropCommon001)
{
	// テスト用プロファイル名
	const auto profileName(GetParam());

	// コントロールプロセスを起動する
	const auto dwControlProcessId = testing::CreateControlProcess(profileName);
	EXPECT_THAT(dwControlProcessId, Ne(0));

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName);
	EXPECT_THAT(hTrayWnd, NotNull());

	const std::jthread t([hTrayWnd] {
		cxx::com_pointer<ITrayWnd> pDispatch = nullptr;
		EXPECT_HRESULT_SUCCEEDED(::AccessibleObjectFromWindow(
			hTrayWnd,
			OBJID_NATIVEOM,
			IID_PPV_ARGS(&pDispatch)
		));

		// トレイアイコン右クリックメニューを表示させる
		DISPID dispid = 2; // ShowTrayContextMenu
		DISPPARAMS params = {};
		EXPECT_HRESULT_SUCCEEDED(pDispatch->Invoke(
			dispid,
			IID_NULL,
			LOCALE_USER_DEFAULT,
			DISPATCH_METHOD,
			&params,
			nullptr,
			nullptr,
			nullptr
		));
	});

	EmulateSelectPopupMenu(L"共通設定(C)...");

	// 共通設定ダイアログが表示されるのを待って閉じる
	const auto hWndDlgPropCommon = WaitForDialog(LS(STR_PROPCOMMON));
	EmulateInvokeButton(hWndDlgPropCommon, L"OK");

	// コントロールプロセスに終了指示を出して終了を待つ
	testing::TerminateControlProcess(profileName, dwControlProcessId);
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

} // namespace winmain
