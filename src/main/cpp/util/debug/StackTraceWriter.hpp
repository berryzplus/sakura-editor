/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "util/debug/DbgHelp.hpp"

#include <Windows.h>
#include <DbgHelp.h>

#include <ostream>

namespace cxx {

/*!
 * @brief スタックトレースを出力するクラス
 */
class StackTraceWriter final
{
private:
	using Base = TSakuraSingleton<StackTraceWriter>;
	using Me = StackTraceWriter;

	static void _CleanUp(
		const StackTraceWriter* pThis
	) noexcept;
	using Holder = cxx::ResourceHolder<&_CleanUp>;

public:
	StackTraceWriter();

	void WriteStackTrace(
		CONTEXT& context,
		std::ostream* out
	) const;

private:
	bool SymCleanup(
			_In_ HANDLE hProcess
	) const noexcept;

	bool SymInitialize(
		_In_ HANDLE hProcess,
		_In_opt_ PCSTR UserSearchPath,
		_In_ bool fInvadeProcess
	) const noexcept;

	DWORD SymSetOptions(
		_In_ DWORD   SymOptions
	) const noexcept;

	HANDLE m_Process = ::GetCurrentProcess();
	HANDLE m_Thread = ::GetCurrentThread();

	DbgHelp m_DbgHelp{};

	decltype(&::StackWalk64) m_pfnStackWalk64 = nullptr;
	decltype(&::SymCleanup) m_pfnSymCleanup = nullptr;
	decltype(&::SymFromAddr) m_pfnSymFromAddr = nullptr;
	decltype(&::SymFunctionTableAccess64) m_pfnSymFunctionTableAccess64 = nullptr;
	decltype(&::SymGetModuleBase64) m_pfnSymGetModuleBase64 = nullptr;
	decltype(&::SymInitialize) m_pfnSymInitialize = nullptr;
	decltype(&::SymSetOptions) m_pfnSymSetOptions = nullptr;

	Holder m_Holder{ this };
};

}	// namespace cxx
