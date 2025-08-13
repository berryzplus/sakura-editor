/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

namespace cxx_util {

/*!
 * ワイド文字列をマルチバイト文字列に変換する
 */
std::string wcstombs_s(std::wstring_view wcs);

} // end of namespace cxx_util
