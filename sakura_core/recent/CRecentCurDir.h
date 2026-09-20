/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTCURDIR_A5846FA5_5608_4E6A_9A57_65DE2133E40A_H_
#define SAKURA_CRECENTCURDIR_A5846FA5_5608_4E6A_9A57_65DE2133E40A_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CCurDirString = StaticString<_MAX_PATH>;

//! コマンドの履歴を管理 (RECENT_FOR_CUR_DIR)
class CRecentCurDir final : public CRecentStringImp<CCurDirString, true>{
public:
	//生成
	CRecentCurDir();

	//オーバーライド
	const WCHAR*	GetItemText( int nIndex ) const override;
};

#endif /* SAKURA_CRECENTCURDIR_A5846FA5_5608_4E6A_9A57_65DE2133E40A_H_ */
