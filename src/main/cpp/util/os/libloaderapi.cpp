/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/os/libloaderapi.hpp"

#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"

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
)
{
	// DLLシンボルのアドレスを取得する
	const auto addr = Kernel32::getInstance()->GetProcAddress(hModule, procName.c_str());

	// 戻り値が NULL だったら例外を投げる
	if (!addr) {
		throw std::system_error(
			std::make_error_code(std::errc::not_supported),
			"GetProcAddress() failed"
		);
	}

	return addr;
}

/*!
 * @brief 外部DLLを読み込む
 *
 * 指定したモジュールを呼出元プロセスのプロセス空間にマップします。
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
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _FileName{ fileName };

	// NUL終端文字列を取り出す
	const auto lpFileName = _FileName.c_str();

	// パラメーターチェック
	if (!lpFileName || !lpFileName[0]) {
		// この例外が出る場合、使い方が誤っているので、呼出元を修正すること。
		throw std::invalid_argument("fileName is required.");
	}

	const auto hModule = Kernel32::getInstance()->LoadLibraryExW(
		lpFileName,
		hFile,
		dwFlags
	);

	if (!hModule) {
		throw cxx::make_system_error("LoadLibraryExW() failed");
	}

	return hModule;
}

}	// namespace cxx
