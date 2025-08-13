/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

// プロジェクトのプリコンパイル済みヘッダーを参照する
#include "StdAfx.h"

// テストではGMockを使う前提にする
#include <gmock/gmock.h>

// テストで使う標準C++ヘッダー（追加するときは昇順で。）
#include <fstream>

// マッチャーのusing（追加するときは昇順で。）
using ::testing::_;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::Lt;
using ::testing::Ne;
using ::testing::Return;
using ::testing::StrCaseEq;
using ::testing::StrCaseNe;
using ::testing::StrEq;
using ::testing::StrNe;
using ::testing::ThrowsMessage;

#if defined(_MSC_VER)
#  define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#  define NORETURN __attribute__((noreturn))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define NORETURN _Noreturn
#else
#  define NORETURN
#endif
