/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/debug/DbgHelp.hpp"

#include "util/os/make_system_error.hpp"

namespace cxx {

// プロセスのシンボル ハンドラーを初期化します。
DbgHelp::DbgHelp()
{
	// DbgHelp.DLLを読み込む
	const auto hDllDbgHelp = ::LoadLibraryW(L"DbgHelp.dll");

	// 読めなかったら動作できない。（実質、考慮しない。）
	if (!hDllDbgHelp) throw cxx::make_system_error("LoadLibraryW() failed");

	reset(hDllDbgHelp);
}

}	// namespace cxx
