﻿/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "util/StaticType.h"

#include "_main/CNormalProcess.h"

/*!
	@brief StaticVectorのテスト
 */
TEST(StaticVector, push_back)
{
	// メモリ確保失敗時に表示するメッセージボックスで、
	// 「アプリ名」を取得するためにプロセスのインスタンスが必要。
	CNormalProcess cProcess(::GetModuleHandle(nullptr), L"");

	// サイズ1の配列を用意する
	auto vec = StaticVector<long long, 1>();
	const auto& constVec = vec;
	EXPECT_EQ(0, vec.size());
	EXPECT_EQ(1, vec.max_size());

	// 1つめのデータを登録する
	vec.push_back(0xabcdef);
	EXPECT_EQ(1, vec.size());
	EXPECT_EQ(0xabcdef, vec[0]);
	EXPECT_EQ(0xabcdef, constVec[0]);

	// 飽和したのでこれ以上追加できない
	EXPECT_EQ(vec.max_size(), vec.size());

	// 追加しようとしてもできないことを確認する

#ifdef _DEBUG
	// デバッグビルドでは、正常にクラッシュする
	EXPECT_DEATH({ vec.push_back(0xffffff); }, "");
#else
	// リリースビルドでもクラッシュする
	EXPECT_THROW({ vec.push_back(0xffffff); }, std::out_of_range);
#endif

	// 追加できないので、サイズをカウントアップしてはいけない
	EXPECT_EQ(1, vec.size());
}
