/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"

#include <Windows.h>
#include <DbgHelp.h>

namespace cxx {

class FileHandle;

/*!
 * @brief ミニダンプを出力するクラス
 */
class MiniDumpWriter final
{
public:
	void WriteMiniDump(
		std::wstring_view file,
		PEXCEPTION_POINTERS pExceptionParams
	) const;

	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	void WriteMiniDump(
		const A& file,
		PEXCEPTION_POINTERS pExceptionParams
	) const
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _File{ file };

		// 文字列を渡すバージョンを呼び出す
		WriteMiniDump(
			_File.str(),
			pExceptionParams
		);
	}

private:
	bool _MiniDumpWriteDump(
        const FileHandle& hFile,
		_In_ MINIDUMP_TYPE DumpType,
		const std::optional<PMINIDUMP_EXCEPTION_INFORMATION>& optExceptionParams
    ) const;
};

}	// namespace cxx
