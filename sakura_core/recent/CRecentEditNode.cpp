/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "recent/CRecentEditNode.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentEditNode::CRecentEditNode()
{
	Create(
		GetShareData()->m_sNodes.m_pEditArr,
		0,
		&GetShareData()->m_sNodes.m_nEditArrNum,
		nullptr,
		MAX_EDITWINDOWS,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CRecentEditNode::CompareItemData( const EditNode* p1, const EditNode* p2 ) const
{
	return int(p1->m_hWnd - p2->m_hWnd);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   固有インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CRecentEditNode::FindItemByHwnd(HWND hwnd) const
{
	int n = GetItemCount();
	for(int i=0;i<n;i++){
		if (GetItem(i).m_hWnd == hwnd) return i;
	}
	return -1;
}

void CRecentEditNode::DeleteItemByHwnd(HWND hwnd)
{
	int n = FindItemByHwnd(hwnd);
	if(n!=-1){
		DeleteItem(n);
	}
	else{
		DEBUG_TRACE( L"DeleteItemByHwnd失敗\n" );
	}
}
