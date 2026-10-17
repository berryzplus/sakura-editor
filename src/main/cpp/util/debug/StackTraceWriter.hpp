/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "util/design_template.h"

#include <Windows.h>
#include <DbgHelp.h>

#include <ostream>

namespace cxx {

/*!
 * @brief スタックトレースを出力するクラス
 */
class StackTraceWriter : public TSakuraSingleton<StackTraceWriter>
{
private:
	using DbgHelpHolder = cxx::ResourceHolder<&::FreeLibrary>;

	using Base = TSakuraSingleton<StackTraceWriter>;
	using Me = StackTraceWriter;

public:
	static void _CleanUp(
		const StackTraceWriter* pThis
	) noexcept;
	using Holder = cxx::ResourceHolder<&_CleanUp>;

	StackTraceWriter();

	void WriteStackTrace(
		CONTEXT& context,
		std::ostream* out
	) const;

	virtual BOOL StackWalk64(
		_In_ DWORD MachineType,
		_In_ HANDLE hProcess,
		_In_ HANDLE hThread,
		_Inout_ LPSTACKFRAME64 StackFrame,
		_Inout_ PVOID ContextRecord,
		_In_opt_ PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
		_In_opt_ PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
		_In_opt_ PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
		_In_opt_ PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
	) const;

	virtual BOOL SymFromAddr(
		_In_ HANDLE hProcess,
		_In_ DWORD64 Address,
		_Out_opt_ PDWORD64 Displacement,
		_Inout_ PSYMBOL_INFO Symbol
	) const;

	virtual BOOL SymInitialize(
		_In_ HANDLE hProcess,
		_In_opt_ PCSTR UserSearchPath,
		_In_ BOOL fInvadeProcess
	) const;

	virtual DWORD SymSetOptions(
		_In_ DWORD   SymOptions
	) const;

	HANDLE m_Process = ::GetCurrentProcess();
	HANDLE m_Thread = ::GetCurrentThread();

	DbgHelpHolder m_DbgHelp{};

	decltype(&::StackWalk64) m_pfnStackWalk64 = nullptr;
	decltype(&::SymFromAddr) m_pfnSymFromAddr = nullptr;
	decltype(&::SymFunctionTableAccess64) m_pfnSymFunctionTableAccess64 = nullptr;
	decltype(&::SymGetModuleBase64) m_pfnSymGetModuleBase64 = nullptr;
	decltype(&::SymInitialize) m_pfnSymInitialize = nullptr;
	decltype(&::SymSetOptions) m_pfnSymSetOptions = nullptr;

	Holder m_Holder{ this };
};

}	// namespace cxx
