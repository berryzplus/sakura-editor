/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTFILE_11698DF0_9914_4163_8A68_8E611163D2E9_H_
#define SAKURA_CRECENTFILE_11698DF0_9914_4163_8A68_8E611163D2E9_H_
#pragma once

#include "basis/EditInfo.h" //EditInfo
#include "recent/CRecentImp.h"

//! EditInfoの履歴を管理 (RECENT_FOR_FILE)
class CRecentFile final : public CRecentImp<EditInfo, SFilePath> {
public:
	//生成
	CRecentFile();

	//オーバーライド
	SFilePath& GetItemString(int nIndex) override
	{
		return GetItem(nIndex).m_szPath;
	}
	const SFilePath& GetItemString(int nIndex) const override
	{
		return GetItem(nIndex).m_szPath;
	}

	int				CompareItemData( const EditInfo* p1, const EditInfo* p2 ) const override;

	//固有インターフェース
	int FindItemByPath(const WCHAR* pszPath) const;
};

#endif /* SAKURA_CRECENTFILE_11698DF0_9914_4163_8A68_8E611163D2E9_H_ */
