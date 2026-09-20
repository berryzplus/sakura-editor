/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_
#define SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

#include "recent/SShare_History.h"

//! GREPフォルダーの履歴を管理 (RECENT_FOR_GREP_FOLDER)
class CRecentGrepFolder final : public CRecentStringImp<SGrepFolder, false>{
public:
	//生成
	CRecentGrepFolder();
};

#endif /* SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_ */
