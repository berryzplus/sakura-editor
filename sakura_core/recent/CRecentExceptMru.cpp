/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentExceptMru.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExceptMRU::CRecentExceptMRU()
{
	Create(
		GetShareData()->m_sHistory.m_aExceptMRU.dataPtr(),
		GetShareData()->m_sHistory.m_aExceptMRU.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sHistory.m_aExceptMRU._GetSizeRef(),
		nullptr,
		MAX_MRU,
		nullptr
	);
}
