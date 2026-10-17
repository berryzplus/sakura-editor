/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/debug/StackTraceWriter.hpp"

#include "util/os/Kernel32.hpp"

namespace cxx {

#if defined(_M_AMD64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_ARM64)
	constexpr auto machineType = IMAGE_FILE_MACHINE_ARM64;
#elif defined(_M_IX86)
	constexpr auto machineType = IMAGE_FILE_MACHINE_I386;
#endif

HMODULE LoadLibraryExW(
	std::wstring_view fileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
);

template <typename T>
void GetProcAddress(
	_In_ HMODULE hModule,
	_In_z_ LPCSTR lpProcName,
	_Out_ T& pfnProc
)
{
	pfnProc = std::bit_cast<T>(Kernel32::getInstance()->GetProcAddress(hModule, lpProcName));
}

/* static */ void StackTraceWriter::_CleanUp(
	const StackTraceWriter* pThis
) noexcept
{
	decltype(&::SymCleanup) pfnSymCleanup = nullptr;
	cxx::GetProcAddress(pThis->m_DbgHelp.get(), "SymCleanup", pfnSymCleanup);

	(*pfnSymCleanup)(pThis->m_Process);
}

// プロセスのシンボル ハンドラーを初期化します。
StackTraceWriter::StackTraceWriter()
{
	static std::once_flag symInitOnce;

	// DbgHelp.DLLを読み込む
	HMODULE hDllDbgHelp = cxx::LoadLibraryExW(L"dbghelp.dll", nullptr, 0);

	// 読めなかったら例外がスローされるので以下はfailしない
	assert(hDllDbgHelp);

	m_DbgHelp = hDllDbgHelp;

	cxx::GetProcAddress(m_DbgHelp.get(), "StackWalk64", m_pfnStackWalk64);
	cxx::GetProcAddress(m_DbgHelp.get(), "SymFromAddr", m_pfnSymFromAddr);
	cxx::GetProcAddress(m_DbgHelp.get(), "SymFunctionTableAccess64", m_pfnSymFunctionTableAccess64);
	cxx::GetProcAddress(m_DbgHelp.get(), "SymGetModuleBase64", m_pfnSymGetModuleBase64);
	cxx::GetProcAddress(m_DbgHelp.get(), "SymInitialize", m_pfnSymInitialize);
	cxx::GetProcAddress(m_DbgHelp.get(), "SymSetOptions", m_pfnSymSetOptions);

	std::call_once(symInitOnce, [process = m_Process, this] {
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
		PCSTR UserSearchPath = nullptr;
		BOOL  fInvadeProcess = TRUE;
		SymInitialize(process, UserSearchPath, fInvadeProcess);
	});
}

void StackTraceWriter::WriteStackTrace(
	CONTEXT& context,
	std::ostream* out
) const
{
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

	std::array<BYTE, sizeof(SYMBOL_INFO) + (MAX_SYM_NAME + 1) - 1> buffer = {};

	auto symbol          = PSYMBOL_INFO(buffer.data());
	symbol->SizeOfStruct = sizeof(*symbol);
	symbol->MaxNameLen   = MAX_SYM_NAME;

	*out << "PrintStackTrace()";

	// スタック トレースを取得します。
	while (StackWalk64(
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
			SymFromAddr(m_Process, address, nullptr, symbol))
		{
			*out << std::format("\tat {} in {}", std::string(symbol->Name), symbol->Address) << std::endl;
		}
	}

	*out << std::endl;
}

BOOL StackTraceWriter::StackWalk64(
	_In_ DWORD MachineType,
	_In_ HANDLE hProcess,
	_In_ HANDLE hThread,
	_Inout_ LPSTACKFRAME64 StackFrame,
	_Inout_ PVOID ContextRecord,
	_In_opt_ PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
	_In_opt_ PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
	_In_opt_ PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
	_In_opt_ PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
) const
{
	return (*m_pfnStackWalk64)(
		MachineType,
		hProcess,
		hThread,
		StackFrame,
		ContextRecord,
		ReadMemoryRoutine,
		FunctionTableAccessRoutine,
		GetModuleBaseRoutine,
		TranslateAddress
	);
}

BOOL StackTraceWriter::SymFromAddr(
	_In_ HANDLE hProcess,
	_In_ DWORD64 Address,
	_Out_opt_ PDWORD64 Displacement,
	_Inout_ PSYMBOL_INFO Symbol
) const
{
	return (*m_pfnSymFromAddr)(
		hProcess,
		Address,
		Displacement,
		Symbol
	);
}

BOOL StackTraceWriter::SymInitialize(
	_In_ HANDLE hProcess,
	_In_opt_ PCSTR UserSearchPath,
	_In_ BOOL fInvadeProcess
) const
{
	return (*m_pfnSymInitialize)(
		hProcess,
		UserSearchPath,
		fInvadeProcess
	);
}

DWORD StackTraceWriter::SymSetOptions(
	_In_ DWORD   SymOptions
) const
{
	return (*m_pfnSymSetOptions)(
		SymOptions
	);
}

}	// namespace cxx
