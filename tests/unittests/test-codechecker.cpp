﻿/*
	Copyright (C) 2023, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "charset/codechecker.h"

TEST(ConvertToUtf32, SurrogatePair)
{
	// 絵文字　男性のシンボル
	const auto& s = L"\U0001f6b9";
	EXPECT_EQ(0x1f6b9, ConvertToUtf32(s));
}

TEST(ConvertToUtf32, VariationSelector)
{
	// 異体字セレクタ　VS17
	const auto& s = L"\U000e0100";
	EXPECT_EQ(0xe0100, ConvertToUtf32(s));
}

TEST(ConvertToUtf32, BinaryOnSurrogate)
{
	// 独自仕様　変換できない文字は1byteずつ下位サロゲートに詰める
	const auto& s = L"\xdcff";
	EXPECT_EQ(0, ConvertToUtf32(s));
}

TEST(IsVariationSelector, VariationSelectorCheck)
{
	// 異体字セレクタ開始
	const auto& vs1 = L"\U000E0100";
	EXPECT_TRUE(IsVariationSelector(vs1));

	// 異体字セレクタ終了
	const auto& vs2 = L"\U000E01EF";
	EXPECT_TRUE(IsVariationSelector(vs2));

	// 非該当文字列
	const auto& notvs1 = L"";
	EXPECT_FALSE(IsVariationSelector(notvs1));
	const auto& notvs2 = L"\xDB40";
	EXPECT_FALSE(IsVariationSelector(notvs2));
	const auto& notvs3 = L"\U000E00FF";
	EXPECT_FALSE(IsVariationSelector(notvs3));
	const auto& notvs4 = L"\U000E01F0";
	EXPECT_FALSE(IsVariationSelector(notvs4));
}
