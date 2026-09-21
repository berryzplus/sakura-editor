/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "recent/CRecentFile.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFile::CRecentFile()
{
	Create(
		GetShareData()->m_sHistory.m_fiMRUArr,
		int(std::size(GetShareData()->m_sHistory.m_fiMRUArr[0].m_szPath)),
		&GetShareData()->m_sHistory.m_nMRUArrNum,
		GetShareData()->m_sHistory.m_bMRUArrFavorite,
		MAX_MRU,
		&(GetShareData()->m_Common.m_sGeneral.m_nMRUArrNum_MAX)
	);
}

int CRecentFile::CompareItemData( const EditInfo* p1, const EditInfo* p2 ) const
{
	return p1->m_szPath.compare(p2->m_szPath);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   固有インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CRecentFile::FindItemByPath(const WCHAR* pszPath) const
{
	int n = GetItemCount();
	for(int i=0;i<n;i++){
		if (0 == GetItem(i).m_szPath.compare(pszPath)) return i;
	}
	return -1;
}
