/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "basis/message_error.hpp"

#include "cxx_util/wcstombs_s.hpp"

namespace basis {

/*!
 * コンストラクター
 *
 * パラメーター message を現在のコードページに変換して保持する。
 * 変換できない文字を含んでいた場合 what() は空文字列となる。
 * 
 * @param message エラーメッセージ。現在のコードページに変換できない文字を含んではならない。
 */
message_error::message_error(std::wstring_view message)
	: std::runtime_error(cxx_util::wcstombs_s(message))
	, _Message(message)
{
}

} // namespace basis
