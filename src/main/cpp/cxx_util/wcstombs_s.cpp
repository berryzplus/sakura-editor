/*!	@file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "cxx_util/wcstombs_s.hpp"

#include "cxx_util/ResourceHolder.hpp"

namespace cxx_util {

/*!
 * ワイド文字列をマルチバイト文字列に変換する
 */
std::string wcstombs_s(std::wstring_view wcs)
{
	std::string buffer;

	// 現在のスレッドロケールを取得
	using LocaleHolder = ResourceHolder<_locale_t, &_free_locale>;
	if (const LocaleHolder locale = _get_current_locale()) {
		// 変換に必要なバッファサイズを求める
		size_t required = 0;
		if (const auto ret = _wcstombs_s_l(&required, nullptr, 0, std::data(wcs), 0, locale); EILSEQ == ret) {
			throw std::invalid_argument("Invalid wide character sequence.");
		}

		// 変換に必要な出力バッファを確保する
		buffer.resize(required, '\0');

		// ワイド文字列をマルチバイト文字列に変換する
		size_t converted = 0;
		_wcstombs_s_l(&converted, std::data(buffer), std::size(buffer), std::data(wcs), _TRUNCATE, locale);

		buffer.resize(converted - 1); // wcstombs_sの戻り値は終端NULを含むので -1 する
	}

	return buffer;
}

} // end of namespace cxx_util
