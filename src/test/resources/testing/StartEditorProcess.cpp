/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "pch.h"

#include "testing/StartEditorProcess.hpp"

#include "cxx_util/wcstombs_s.hpp"
#include "util/StaticType.h"
#include "CDataProfile.h" // StringBufferW

// MSVCデバッグビルド専用コードを隔離するためのCマクロ
#if defined(_MSC_VER) && defined(_DEBUG) && (defined(_M_AMD64) || defined(_M_IX86))
#define USE_STACK_TRACE
#endif

#ifdef USE_STACK_TRACE

// デバッグ出力の独自関数を使う
#include "debug/Debug1.h"	// DebugOutW, TRACEマクロ等

// Windows SDKのデバッグヘルパーとリンクする
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

#endif

namespace testing {

#ifdef USE_STACK_TRACE

#ifdef _M_AMD64
	constexpr auto machineType = IMAGE_FILE_MACHINE_AMD64;
#else // _M_IX86
	constexpr auto machineType = IMAGE_FILE_MACHINE_I386;
#endif

void PrintStackTrace(
    CONTEXT& context)
{
    const auto process = GetCurrentProcess();
    const auto thread  = GetCurrentThread();

    // Dbghelp.dll
    // プロセスのシンボル ハンドラーを初期化します。
    PCSTR UserSearchPath = nullptr;
    BOOL  fInvadeProcess = TRUE;
    SymInitialize(process, UserSearchPath, fInvadeProcess);

    STACKFRAME64 stack     = {};
    stack.AddrPC.Mode      = AddrModeFlat;
    stack.AddrFrame.Mode   = AddrModeFlat;
    stack.AddrStack.Mode   = AddrModeFlat;

#ifdef _M_AMD64
    stack.AddrPC.Offset    = context.Rip;
    stack.AddrFrame.Offset = context.Rbp;
    stack.AddrStack.Offset = context.Rsp;
#else // _M_IX86
    stack.AddrPC.Offset    = context.Eip;
    stack.AddrFrame.Offset = context.Ebp;
    stack.AddrStack.Offset = context.Esp;
#endif

	std::array<BYTE, sizeof(SYMBOL_INFO) + (MAX_SYM_NAME + 1) * sizeof(TCHAR) - 1> buffer = {};

    auto symbol          = PSYMBOL_INFO(buffer.data());
    symbol->SizeOfStruct = sizeof(*symbol);
    symbol->MaxNameLen   = MAX_SYM_NAME;

    TRACE("PrintStackTrace()");

    // Dbghelp.dll
    // スタック トレースを取得します。
    while (StackWalk64(
        machineType,
        process,
        thread,
        &stack,
        &context,
        nullptr,
        SymFunctionTableAccess64,
        SymGetModuleBase64,
        nullptr))
    {
        if (const auto address = stack.AddrPC.Offset;
            SymFromAddr(process, address, nullptr, symbol))
        {
			DebugOutW(L"\tat %s in %0p\n", symbol->Name, symbol->Address);
        }
    }

	DebugOutW(L"\n");

	// Dbghelp.dll
    // プロセス ハンドルに関連付けられているすべてのリソースの割り当てを解除します。
    SymCleanup(process);
}

LONG WINAPI MyUnhandledExceptionFilter(
    int                 nExceptionCode,
    EXCEPTION_POINTERS* pExceptionInfo)
{
    UNREFERENCED_PARAMETER(nExceptionCode);

	auto& context = *pExceptionInfo->ContextRecord;

    PrintStackTrace(context);

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif

int CallWinMain(
	StringBufferW lpCmdLine
)
{
#ifdef USE_STACK_TRACE
	__try
    {
#endif

		// 実行中モジュールのインスタンスハンドルを取得する
		const auto hInstance = GetModuleHandleW(nullptr);

		return wWinMain(hInstance, nullptr, lpCmdLine, SW_SHOWDEFAULT);

#ifdef USE_STACK_TRACE
    }
    __except (MyUnhandledExceptionFilter(GetExceptionCode(),  GetExceptionInformation()))
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
	std::cout << std::format("{}({}): launching process [{}]", std::source_location::current().file_name(), std::source_location::current().line(), cxx_util::wcstombs_s(command)) << std::endl;

	assert(std::regex_search(command, std::wregex(LR"(-PROF\b)", std::wregex::icase)));

	// wWinMainに渡すためのコマンドライン
	std::wstring buffer(command);

	// コマンドラインに -CODE 指定がない場合は付与する
	if (!std::regex_search(command, std::wregex(LR"(-NOWIN\b)", std::wregex::icase)) &&
		!std::regex_search(command, std::wregex(LR"(-CODE\b)", std::wregex::icase)))
	{
		buffer += L" -CODE=99"sv; // 指定しないとファイル名から文字コードを判定する仕様によりJIS指定になってしまう。
	}

	// wWinMainを起動する(戻り値は0が正常)
	const auto ret = CallWinMain(StringBufferW(buffer.data(), buffer.length() + 1));

	// ログ出力
	std::cout << std::format("{}({}): leaving process   [{}] => {}", std::source_location::current().file_name(), std::source_location::current().line(), cxx_util::wcstombs_s( command), ret) << std::endl << std::endl;

	return ret;
}

} // namespace testing
