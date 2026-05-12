/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_
#define SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_
#pragma once

#include <string>
#include "util/string_ex.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

//共通型
using SFilePath = StaticString<_MAX_PATH>;
using SFilePathLong = StaticString<MAX_GREP_PATH>;
class CFilePath : public StaticString<_MAX_PATH>{
private:
	using Super = StaticString<_MAX_PATH>;
public:
	CFilePath() = default;
	CFilePath(const WCHAR* rhs) : Super(std::wstring_view{ rhs ? rhs : L"" }) {}

	[[nodiscard]] bool IsValidPath() const noexcept { return !empty(); }
	[[nodiscard]] std::wstring GetDirPath() const
	{
		std::filesystem::path path{ *this };
		return path.remove_filename();
	}

	//拡張子を取得する
	[[nodiscard]] LPCWSTR GetExt( bool bWithoutDot = false ) const
	{
		// 文字列の末尾アドレスを取得
		const WCHAR* tail = c_str() + Length();

		// 文字列末尾から逆方向に L'.' を検索
		if (const auto *p = ::wcsrchr(c_str(), L'.')) {
			// L'.'で始まる文字列がパス区切りを含まない場合のみ「拡張子あり」と看做す
			if (const bool hasExt = !::wcspbrk(p, L"\\/"); hasExt && !bWithoutDot) {
				return p;
			}
			else if (hasExt && p < tail) {
				return p + 1;		//bWithoutDot==trueならドットなしを返す
			}
		}

		// 文字列末尾のアドレスを返す
		return tail;
	}
};

#endif /* SAKURA_CMYSTRING_009A2525_6B06_4C1B_B089_C1B8A424A565_H_ */
