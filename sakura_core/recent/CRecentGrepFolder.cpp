/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentGrepFolder.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentGrepFolder::CRecentGrepFolder()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aGrepFolders.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aGrepFolders.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aGrepFolders._GetSizeRef(),
		nullptr,
		MAX_GREPFOLDER,
		nullptr
	);
}
