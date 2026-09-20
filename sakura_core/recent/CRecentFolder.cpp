/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentFolder.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFolder::CRecentFolder()
{
	Create(
		&GetShareData()->m_sHistory.m_szOPENFOLDERArr[0],
		GetShareData()->m_sHistory.m_szOPENFOLDERArr[0].GetBufferCount(),
		&GetShareData()->m_sHistory.m_nOPENFOLDERArrNum,
		GetShareData()->m_sHistory.m_bOPENFOLDERArrFavorite,
		MAX_OPENFOLDER,
		&(GetShareData()->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX)
	);
}
