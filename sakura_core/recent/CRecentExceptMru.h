/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTEXCEPTMRU_4DF7E5C5_2EC1_4A19_B31C_74EF43DC08AE_H_
#define SAKURA_CRECENTEXCEPTMRU_4DF7E5C5_2EC1_4A19_B31C_74EF43DC08AE_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CMetaPath = StaticString<_MAX_PATH>;

//! フォルダーの履歴を管理 (RECENT_FOR_FOLDER)
class CRecentExceptMRU final : public CRecentStringImp<CMetaPath, false>{
public:
	//生成
	CRecentExceptMRU();

	//オーバーライド
	const WCHAR*	GetItemText( int nIndex ) const override;
};

#endif /* SAKURA_CRECENTEXCEPTMRU_4DF7E5C5_2EC1_4A19_B31C_74EF43DC08AE_H_ */
