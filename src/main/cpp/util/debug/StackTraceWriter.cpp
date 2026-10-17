/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/debug/StackTraceWriter.hpp"

#include "util/os/make_system_error.hpp"
#include "util/os.h"

namespace cxx {

/* static */ void StackTraceWriter::_CleanUp(
	const StackTraceWriter* pThis
) noexcept
{
	// 引数がNULLなら何もしない
	if (!pThis) return;

	// DLL初期化に失敗していたら何もしない
	if (!pThis->m_pfnSymCleanup) return;

	pThis->SymCleanup(pThis->m_Process);
}

// プロセスのシンボル ハンドラーを初期化します。
StackTraceWriter::StackTraceWriter()
{
#pragma push_macro("LOAD_SYMBOL")

#define LOAD_SYMBOL(name) \
	m_pfn ## name = m_DbgHelp.LoadSymbol<decltype(m_pfn ## name)>( \
		#name \
	); \
	0 /* ←セミコロンを付けないとエラーになるように。 */

	LOAD_SYMBOL(StackWalk64);
	LOAD_SYMBOL(SymCleanup);
	LOAD_SYMBOL(SymFromAddr);
	LOAD_SYMBOL(SymFunctionTableAccess64);
	LOAD_SYMBOL(SymGetModuleBase64);
	LOAD_SYMBOL(SymInitialize);
	LOAD_SYMBOL(SymSetOptions);

#pragma pop_macro("LOAD_SYMBOL")

	SymSetOptions(
		SYMOPT_LOAD_LINES | SYMOPT_UNDNAME
	);

	PCSTR UserSearchPath = nullptr;
	BOOL  fInvadeProcess = TRUE;
	SymInitialize(
		m_Process,
		UserSearchPath,
		fInvadeProcess
	);
}

void StackTraceWriter::WriteStackTrace(
	CONTEXT& context,
	std::ostream* out
) const
{
#if defined(_M_AMD64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_ARM64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_ARM64;
#elif defined(_M_IX86)
	constexpr auto machineType = IMAGE_FILE_MACHINE_I386;
#endif

	STACKFRAME64 stack     = {};
	stack.AddrPC.Mode      = AddrModeFlat;
	stack.AddrFrame.Mode   = AddrModeFlat;
	stack.AddrStack.Mode   = AddrModeFlat;

#ifdef _M_AMD64
	stack.AddrPC.Offset    = context.Rip;
	stack.AddrFrame.Offset = context.Rbp;
	stack.AddrStack.Offset = context.Rsp;
#elif defined(_M_ARM64)
	stack.AddrPC.Offset    = context.Pc;
	stack.AddrFrame.Offset = context.Fp;
	stack.AddrStack.Offset = context.Sp;
#else // _M_IX86
	stack.AddrPC.Offset    = context.Eip;
	stack.AddrFrame.Offset = context.Ebp;
	stack.AddrStack.Offset = context.Esp;
#endif

	std::array<BYTE, sizeof(SYMBOL_INFO) + (MAX_SYM_NAME + 1) - 1> buffer = {};

	auto symbol          = PSYMBOL_INFO(buffer.data());
	symbol->SizeOfStruct = sizeof(*symbol);
	symbol->MaxNameLen   = MAX_SYM_NAME;

	*out << "PrintStackTrace()";

	// スタック トレースを取得します。
	while ((*m_pfnStackWalk64)(
		machineType,
		m_Process,
		m_Thread,
		&stack,
		&context,
		nullptr,
		m_pfnSymFunctionTableAccess64,
		m_pfnSymGetModuleBase64,
		nullptr))
	{
		if (const auto address = stack.AddrPC.Offset;
			(*m_pfnSymFromAddr)(m_Process, address, nullptr, symbol))
		{
			*out << std::format("\tat {} in {}", std::string(symbol->Name), symbol->Address) << std::endl;
		}
	}

	*out << std::endl;
}

bool StackTraceWriter::SymCleanup(
	_In_ HANDLE hProcess
) const noexcept
{
	return (*m_pfnSymCleanup)(
		hProcess
	);
}

bool StackTraceWriter::SymInitialize(
	_In_ HANDLE hProcess,
	_In_opt_ PCSTR UserSearchPath,
	_In_ bool fInvadeProcess
) const noexcept
{
	return (*m_pfnSymInitialize)(
		hProcess,
		UserSearchPath,
		fInvadeProcess
	);
}

DWORD StackTraceWriter::SymSetOptions(
	_In_ DWORD   SymOptions
) const noexcept
{
	return (*m_pfnSymSetOptions)(
		SymOptions
	);
}

}	// namespace cxx
