/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTEDITNODE_51FF7E34_DFF5_45BA_AB77_7845F21F7A85_H_
#define SAKURA_CRECENTEDITNODE_51FF7E34_DFF5_45BA_AB77_7845F21F7A85_H_
#pragma once

#include "env/CAppNodeManager.h"	// EditNode
#include "recent/CRecentImp.h"

//! EditNode(ウィンドウリスト)の履歴を管理 (RECENT_FOR_EDITNODE)
class CRecentEditNode final : public CRecentImp<EditNode, SFilePath> {
public:
	//生成
	CRecentEditNode();

	//オーバーライド
	SFilePath& GetItemString(int nIndex) override
	{
		return GetItem(nIndex).m_szFilePath;
	}
	const SFilePath& GetItemString(int nIndex) const override
	{
		return GetItem(nIndex).m_szFilePath;
	}

	int				CompareItemData( const EditNode* p1, const EditNode* p2 ) const override;

	//固有インターフェース
	int FindItemByHwnd(HWND hwnd) const;
	void DeleteItemByHwnd(HWND hwnd);
};

#endif /* SAKURA_CRECENTEDITNODE_51FF7E34_DFF5_45BA_AB77_7845F21F7A85_H_ */
