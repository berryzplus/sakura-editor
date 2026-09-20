/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTFOLDER_E26A46E2_C8DF_4228_A0D6_24A2712392E9_H_
#define SAKURA_CRECENTFOLDER_E26A46E2_C8DF_4228_A0D6_24A2712392E9_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

#include "recent/SShare_History.h"

//! フォルダーの履歴を管理 (RECENT_FOR_FOLDER)
class CRecentFolder final : public CRecentImp<SPathString, LPCWSTR>{
public:
	//生成
	CRecentFolder();
};

#endif /* SAKURA_CRECENTFOLDER_E26A46E2_C8DF_4228_A0D6_24A2712392E9_H_ */
