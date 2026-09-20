/*! @file

	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentExcludeFile.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExcludeFile::CRecentExcludeFile()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aExcludeFiles.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aExcludeFiles.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aExcludeFiles._GetSizeRef(),
		nullptr,
		MAX_EXCLUDEFILE,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザー管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentExcludeFile::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

