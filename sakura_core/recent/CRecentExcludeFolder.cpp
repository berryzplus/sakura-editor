/*! @file

	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentExcludeFolder.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExcludeFolder::CRecentExcludeFolder()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aExcludeFolders.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aExcludeFolders.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aExcludeFolders._GetSizeRef(),
		nullptr,
		MAX_EXCLUDEFOLDER,
		nullptr
	);
}
