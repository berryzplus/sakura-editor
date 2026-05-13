/*!	@file
	@brief プロセス基底クラス

	@author aroka
	@date 2002/01/07 作成
	@date 2002/01/17 修正
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2004, Moca
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcess.h"

#include "config/app_constants.h"
#include "config/build_config.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "util/module.h"
#include "util/tchar_convert.h"

#include "CSelectLang.h"

// Windows SDKのデバッグヘルパーとリンクする
#pragma comment(lib, "dbghelp.lib")

namespace debug {

#if defined(_M_AMD64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_ARM64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_ARM64;
#elif defined(_M_IX86)
	constexpr auto machineType = IMAGE_FILE_MACHINE_I386;
#endif

// Dbghelp.dll
// プロセスのシンボル ハンドラーを初期化します。
void InitDbgSymbols(HANDLE process)
{
	static std::once_flag symInitOnce;

	std::call_once(symInitOnce, [process] {
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
		PCSTR UserSearchPath = nullptr;
		BOOL  fInvadeProcess = TRUE;
		::SymInitialize(process, UserSearchPath, fInvadeProcess);
	});
}

void PrintStackTraceFromContext(CONTEXT& context)
{
	const auto process = ::GetCurrentProcess();
	const auto thread  = ::GetCurrentThread();

	// プロセスのシンボル ハンドラーを初期化します。
	InitDbgSymbols(process);

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

	std::wcout << L"PrintStackTrace()";

	// Dbghelp.dll
	// スタック トレースを取得します。
	while (::StackWalk64(
		machineType,
		process,
		thread,
		&stack,
		&context,
		nullptr,
		::SymFunctionTableAccess64,
		::SymGetModuleBase64,
		nullptr))
	{
		if (const auto address = stack.AddrPC.Offset;
			::SymFromAddr(process, address, nullptr, symbol))
		{
			std::wcout << std::format(L"\tat {} in {}", cxx::to_wstring(symbol->Name), symbol->Address) << std::endl;
		}
	}

	std::wcout << std::endl;
}

// SEH例外コードを人間が読める文字列に変換する
std::string_view SehExceptionCodeName(DWORD code) noexcept
{
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:         return "access violation (seg fault)";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "array bounds exceeded";
	case EXCEPTION_DATATYPE_MISALIGNMENT:    return "datatype misalignment";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "float: divide by zero";
	case EXCEPTION_FLT_OVERFLOW:             return "float: overflow";
	case EXCEPTION_FLT_STACK_CHECK:          return "float: stack check";
	case EXCEPTION_FLT_UNDERFLOW:            return "float: underflow";
	case EXCEPTION_ILLEGAL_INSTRUCTION:      return "illegal instruction";
	case EXCEPTION_IN_PAGE_ERROR:            return "in-page error";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "integer: divide by zero";
	case EXCEPTION_INT_OVERFLOW:             return "integer: overflow";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "non-continuable exception";
	case EXCEPTION_PRIV_INSTRUCTION:         return "privileged instruction";
	case EXCEPTION_STACK_OVERFLOW:           return "stack overflow";
	default:                                 return {};
	}
}

void DumpSehException(EXCEPTION_POINTERS* ep)
{
	if (!ep || !ep->ExceptionRecord) {
		std::cerr << "no SEH exception info\n";
		return;
	}

	const auto code = ep->ExceptionRecord->ExceptionCode;
	if (const auto name = SehExceptionCodeName(code); !name.empty()) {
		std::cerr << "SEH exception: " << name << "\n";
	}
	std::cerr << "SEH code: 0x"
		<< std::hex << code << std::dec << "\n";
	std::cerr << "SEH address: 0x"
		<< std::hex
		<< uint64_t(ep->ExceptionRecord->ExceptionAddress)
		<< std::dec << "\n";
}

// 未処理例外フィルター: SEH例外またはC++例外をダンプし、スタックトレースを出力
// コンパイルオプション(/EHaなど)によってはC++例外もこのフィルターで捕捉される
LONG OnUnhandledException(EXCEPTION_POINTERS* ep)
{
	DumpSehException(ep);
	if (ep && ep->ContextRecord) {
		PrintStackTraceFromContext(*ep->ContextRecord);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

} // namespace debug

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
*/
CProcess::CProcess(
	HINSTANCE				hInstance,		//!< handle to process instance
	CCommandLineHolder&&	pCommandLine	//!< pointer to command line
)
	: m_hInstance(hInstance)
	, m_pCommandLine(std::move(pCommandLine))
{
}

/*!
	@brief iniファイルパスを取得する
 */
std::filesystem::path CProcess::GetIniFileName() const
{
	if (m_cShareData.IsPrivateSettings()) {
		const DLLSHAREDATA *pShareData = &GetDllShareData();
		return pShareData->m_szPrivateIniFile.c_str();
	}
	return GetExeFileName().replace_extension(L".ini");
}

/*!
	@brief プロセス実行
	
	@author aroka
	@date 2002/01/16
 */
int CProcess::Run(int nCmdShow)
{
	if (!InitializeProcess(nCmdShow)) {

		return 0L;
	}

#ifdef USE_STACK_TRACE

	__try
	{

#endif

		MainLoop();

		return 0L;

#ifdef USE_STACK_TRACE
	}
	__except (debug::OnUnhandledException(GetExceptionInformation()))
	{
		return 1;
	}
#endif
}

/*!
	言語選択後に共有メモリ内の文字列を更新する
*/
void CProcess::RefreshString()
{
	m_cShareData.RefreshString();
}
