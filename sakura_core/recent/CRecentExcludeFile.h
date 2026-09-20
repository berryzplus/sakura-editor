/*! @file

	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_
#define SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

#include "recent/SShare_History.h"

//! Excludeファイルの履歴を管理 (RECENT_FOR_Exclude_FILE)
class CRecentExcludeFile final : public CRecentStringImp<SExcludeFile, false>{
public:
	//生成
	CRecentExcludeFile();
};

#endif /* SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_ */
