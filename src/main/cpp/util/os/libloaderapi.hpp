/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "basis/primitive.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

#include <Windows.h>

namespace cxx {

/*!
 * @brief DLLシンボルのアドレスを取得する
 *
 * @param[in] hModule DLLのハンドル
 * @param[in] procName シンボル名
 * @return DLLシンボルのアドレス
 */
FARPROC GetProcAddress(
	_In_ HMODULE hModule,
	const std::string& procName
);

/*!
 * @brief 外部DLLを読み込む
 *
 * @param[in] fileName DLLのパス
 * @param[opt] hFile nullptrを指定します
 * @param[in] DLL読み込みのフラグ
 * @return DLLのハンドル
 */
HMODULE LoadLibraryExW(
	std::wstring_view fileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
);

/*!
 * @brief 外部DLLを読み込む
 *
 * @param[in] fileName DLLのパス
 * @param[opt] hFile nullptrを指定します
 * @param[in] DLL読み込みのフラグ
 * @return DLLのハンドル
 */
template <basis::NullTerminatedStringConstructible<WCHAR> A>
	requires (!std::is_same_v<A, std::wstring_view>)
HMODULE LoadLibraryExW(
	const A& fileName,
	_Reserved_ HANDLE hFile,
	_In_ DWORD dwFlags
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _FileName{ fileName };

	// 文字列を渡すバージョンを呼び出す
	return LoadLibraryExW(
		_FileName.str(),
		hFile,
		dwFlags
	);
}

}	// namespace cxx
