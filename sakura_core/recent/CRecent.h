/*!	@file
	@brief 最近使ったリスト

	お気に入りを含む最近使ったリストを管理する。

	@author MIK
	@date Apr. 05, 2003
	@date Apr. 03, 2005

	@date Oct. 19, 2007 kobake 型チェックが働くように、再設計
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENT_F4D70310_9FAF_4F07_9431_2B011A47142D_H_
#define SAKURA_CRECENT_F4D70310_9FAF_4F07_9431_2B011A47142D_H_
#pragma once

#include "env/DLLSHAREDATA.h"

class CRecent{
public:
	virtual ~CRecent() = default;

	//インスタンス管理
	virtual void	Terminate() = 0;

	//アイテム
	virtual LPCWSTR	GetItemText(int nIndex) const = 0;
	virtual int		GetArrayCount() const = 0;
	virtual int		GetItemCount() const = 0;
	virtual void	DeleteAllItem() = 0;
	virtual bool	DeleteItemsNoFavorite() = 0;
	virtual bool	DeleteItem(int nIndex) = 0;	//!< アイテムをクリア
	virtual bool	AppendItemText(LPCWSTR pszText) = 0;
	virtual bool	EditItemText(int nIndex, LPCWSTR pszText) = 0;
	virtual size_t	GetTextMaxLength() const = 0;

	virtual int		FindItemByText(LPCWSTR pszText) const = 0;

	//お気に入り
	virtual bool	SetFavorite( int nIndex, bool bFavorite = true ) = 0;	//!< お気に入りに設定
	virtual bool	IsFavorite(int nIndex) const = 0;						//!< お気に入りか調べる

	//その他
	virtual int		GetViewCount() const = 0;
	virtual bool	UpdateView() = 0;
};

#endif /* SAKURA_CRECENT_F4D70310_9FAF_4F07_9431_2B011A47142D_H_ */
