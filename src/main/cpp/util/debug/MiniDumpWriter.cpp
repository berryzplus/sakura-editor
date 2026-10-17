/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/debug/MiniDumpWriter.hpp"

#include "util/file/FileHandle.hpp"
#include "util/os/Kernel32.hpp"

namespace cxx {

HMODULE LoadLibraryExW(
	std::wstring_view fileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
);

/*!
 * @brief クラッシュダンプ
 *
 * @author ryoji
 * @date 2009/01/21 新規作成
 * @date 2026/10/16 berryzplus 復刻
 */
void MiniDumpWriter::WriteMiniDump(
	std::wstring_view file,
	PEXCEPTION_POINTERS pExceptionParams
) const
{
	// ファイルを作る
	const auto hFile = FileHandle::CreateFileW(
		file,
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		nullptr
	);

	MINIDUMP_EXCEPTION_INFORMATION exceptionParams{};
	exceptionParams.ThreadId = ::GetCurrentThreadId();
	exceptionParams.ExceptionPointers = pExceptionParams;
	exceptionParams.ClientPointers = FALSE;

	const auto optExceptionParams = pExceptionParams
		? std::make_optional(&exceptionParams)
		: std::nullopt;

	// ミニダンプを出力する
	_MiniDumpWriteDump(
		hFile,
		MiniDumpNormal,
		optExceptionParams
	);
}

bool MiniDumpWriter::_MiniDumpWriteDump(
    const FileHandle& hFile,
	_In_ MINIDUMP_TYPE DumpType,
	const std::optional<PMINIDUMP_EXCEPTION_INFORMATION>& optExceptionParams
) const
{
	// DbgHelp.DLLを読み込む
	HMODULE hDllDbgHelp = cxx::LoadLibraryExW(L"dbghelp.dll", nullptr, 0);

	// 読めなかったら例外がスローされるので以下はfailしない
	assert(hDllDbgHelp);

	using Holder = cxx::ResourceHolder<&::FreeLibrary>;
	Holder dbgHelp{ hDllDbgHelp };

	decltype(&::MiniDumpWriteDump) pfnMiniDumpWriteDump = nullptr;

	pfnMiniDumpWriteDump = std::bit_cast<decltype(pfnMiniDumpWriteDump)>(Kernel32::getInstance()->GetProcAddress(dbgHelp, "MiniDumpWriteDump"));

	const auto hProcess = ::GetCurrentProcess();
	const auto ProcessId = ::GetCurrentProcessId();

	const PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam = nullptr;
	const PMINIDUMP_CALLBACK_INFORMATION CallbackParam = nullptr;

	return (*pfnMiniDumpWriteDump)(
		hProcess,
		ProcessId,
		hFile.get(),
		DumpType,
		optExceptionParams.value_or(nullptr),
		UserStreamParam,
		CallbackParam
	);
}

}	// namespace cxx
