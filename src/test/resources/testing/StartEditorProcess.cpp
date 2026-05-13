/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "pch.h"

#include "testing/StartEditorProcess.hpp"

#include "config/build_config.h"
#include "util/StaticType.h"
#include "util/tchar_convert.h"
#include "env/CDataProfile.h" // StringBufferW

// デバッグ出力の独自関数を使う
#include "debug/Debug1.h"	// DebugOutW, TRACEマクロ等

namespace debug {

LONG OnUnhandledException(EXCEPTION_POINTERS* ep);
void PrintStackTraceFromContext(CONTEXT& context);

} // namespace debug

namespace testing {

void DumpCppException(std::exception_ptr eptr)
{
	if (!eptr) {
		std::cerr << "no active C++ exception\n";
		return;
	}

	try {
		std::rethrow_exception(eptr);
	}
	catch (const _com_error& e) {
		std::wcerr << L"_com_error\n";
		std::wcerr << L"  HRESULT      : 0x" << std::hex
			<< static_cast<unsigned long>(e.Error()) << std::dec << L"\n";
		std::wcerr << L"  ErrorMessage : " << e.ErrorMessage() << L"\n";
		if (e.Description().length() > 0) {
			std::wcerr << L"  Description  : " << static_cast<const wchar_t*>(e.Description()) << L"\n";
		}
		if (e.Source().length() > 0) {
			std::wcerr << L"  Source       : " << static_cast<const wchar_t*>(e.Source()) << L"\n";
		}
	}
	catch (const std::exception& e) {
		std::cerr << "std::exception: " << e.what() << "\n";
	}
	catch (...) {
		std::cerr << "unknown C++ exception\n";
	}
}

// C++例外ハンドラー: set_terminate で登録する
[[noreturn]] void OnTerminate()
{
	if (auto cppEp = std::current_exception()) {
		DumpCppException(cppEp);
	}

	CONTEXT ctx{};
	::RtlCaptureContext(&ctx);
	debug::PrintStackTraceFromContext(ctx);

	std::abort();
}

int CallWinMain(
	std::span<WCHAR> cmdLine
)
{
#ifdef USE_STACK_TRACE

	// terminate対策でスレッドトレースを出すようにする
	std::set_terminate(testing::OnTerminate);

	__try
	{
#endif

		// 実行中モジュールのインスタンスハンドルを取得する
		const auto hInstance = ::GetModuleHandleW(nullptr);

		return wWinMain(hInstance, nullptr, std::data(cmdLine), SW_SHOWDEFAULT);

#ifdef USE_STACK_TRACE
	}
	__except (debug::OnUnhandledException(GetExceptionInformation()))
	{
		return 1;
	}
#endif
}

/*!
 * テストコード専用wWinMain呼出のラッパー関数
 *
 * 単体テストから wWinMain を呼び出すためのラッパー関数です。
 * コマンドラインには -PROF 指定が含まれている必要があります。
 *
 * @param[in] command コマンドライン文字列(exeパスを含まない)
 * @retval 0		正常終了
 * @retval 1		異常終了(asset失敗でabortした場合はこの値)
 * @retval その他	おそらくバグ。サクラエディタは0以外の終了コードを定義していない。
 */
int StartEditorProcess(const std::wstring& command)
{
	// ログ出力
	std::cout << std::format("{}({}): launching process [{}]", std::source_location::current().file_name(), std::source_location::current().line(), cxx::to_string(command)) << std::endl;

	assert(std::regex_search(command, std::wregex(LR"(-PROF\b)", std::wregex::icase)));

	// wWinMainに渡すためのコマンドライン
	std::wstring buffer(command);

	// コマンドラインに -CODE 指定がない場合は付与する
	if (!std::regex_search(command, std::wregex(LR"(-CODE\b)", std::wregex::icase)) &&
		!std::regex_search(command, std::wregex(LR"(-NOWIN\b)", std::wregex::icase)))
	{
		buffer += std::format(LR"( -CODE={})", static_cast<int>(CODE_AUTODETECT)); // 指定しないとファイル名から文字コードを判定する仕様によりJIS指定になってしまう。
	}

	// wWinMainを起動する(戻り値は0が正常)
	const auto ret = CallWinMain(buffer);

	// ログ出力
	std::cout << std::format("{}({}): leaving process   [{}] => {}", std::source_location::current().file_name(), std::source_location::current().line(), cxx::to_string(command), ret) << std::endl << std::endl;

	return ret;
}

} // namespace testing
