/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTGREPFILE_E23BE08A_1B53_492D_85EE_4370AA956BB5_H_
#define SAKURA_CRECENTGREPFILE_E23BE08A_1B53_492D_85EE_4370AA956BB5_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

using CGrepFileString = StaticString<MAX_GREP_PATH>;

//! GREPファイルの履歴を管理 (RECENT_FOR_GREP_FILE)
class CRecentGrepFile final : public CRecentStringImp<CGrepFileString, false>{
public:
	//生成
	CRecentGrepFile();
};

#endif /* SAKURA_CRECENTGREPFILE_E23BE08A_1B53_492D_85EE_4370AA956BB5_H_ */
