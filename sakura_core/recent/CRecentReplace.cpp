/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentReplace.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentReplace::CRecentReplace()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aReplaceKeys.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aReplaceKeys.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aReplaceKeys._GetSizeRef(),
		nullptr,
		MAX_REPLACEKEY,
		nullptr
	);
}
