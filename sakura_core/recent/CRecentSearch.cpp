/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentSearch.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentSearch::CRecentSearch()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aSearchKeys.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aSearchKeys.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aSearchKeys._GetSizeRef(),
		nullptr,
		MAX_SEARCHKEY,
		nullptr
	);
}
