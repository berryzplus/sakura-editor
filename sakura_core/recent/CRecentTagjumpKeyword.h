/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTTAGJUMPKEYWORD_1416AC30_3714_4760_A313_76588D26A0A1_H_
#define SAKURA_CRECENTTAGJUMPKEYWORD_1416AC30_3714_4760_A313_76588D26A0A1_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

#include "recent/SShare_History.h"

//! タグジャンプキーワードの履歴を管理 (RECENT_FOR_TAGJUMP_KEYWORD)
class CRecentTagjumpKeyword final : public CRecentStringImp<STagjumpKeyword, true>{
public:
	//生成
	CRecentTagjumpKeyword();
};

#endif /* SAKURA_CRECENTTAGJUMPKEYWORD_1416AC30_3714_4760_A313_76588D26A0A1_H_ */
