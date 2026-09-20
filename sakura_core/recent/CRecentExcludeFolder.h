/*! @file

	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTEXCLUDEFOLDER_D933B071_8956_4B13_A01D_A5075CCE2A05_H_
#define SAKURA_CRECENTEXCLUDEFOLDER_D933B071_8956_4B13_A01D_A5075CCE2A05_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

#include "recent/SShare_History.h"

//! Excludeフォルダーの履歴を管理 (RECENT_FOR_Exclude_FOLDER)
class CRecentExcludeFolder final : public CRecentImp<SExcludeFolder, LPCWSTR>{
public:
	//生成
	CRecentExcludeFolder();
};

#endif /* SAKURA_CRECENTEXCLUDEFOLDER_D933B071_8956_4B13_A01D_A5075CCE2A05_H_ */
