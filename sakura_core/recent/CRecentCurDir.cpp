/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CRecentCurDir.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentCurDir::CRecentCurDir()
{
	Create(
		GetShareData()->m_sHistory.m_aCurDirs.dataPtr(),
		GetShareData()->m_sHistory.m_aCurDirs.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sHistory.m_aCurDirs._GetSizeRef(),
		nullptr,
		MAX_CMDARR,
		nullptr
	);
}
