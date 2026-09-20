/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentTagjumpKeyword.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentTagjumpKeyword::CRecentTagjumpKeyword()
{
	Create(
		GetShareData()->m_sTagJump.m_aTagJumpKeywords.dataPtr(),
		GetShareData()->m_sTagJump.m_aTagJumpKeywords.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sTagJump.m_aTagJumpKeywords._GetSizeRef(),
		nullptr,
		MAX_TAGJUMP_KEYWORD,
		nullptr
	);
}
