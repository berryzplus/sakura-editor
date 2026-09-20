/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CTAGJUMPMANAGER_487A43FC_EB78_44CF_B1E4_6FD78EF1F35A_H_
#define SAKURA_CTAGJUMPMANAGER_487A43FC_EB78_44CF_B1E4_6FD78EF1F35A_H_
#pragma once

#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "env/CSakuraEnvironment.h"	//env::ShareDataClient
#include "recent/SShare_History.h"
#include "util/StaticType.h"

using STagjumpKeyword = StaticString<_MAX_PATH, true>;

// 2004/06/21 novice タグジャンプ機能追加
//! タグジャンプ情報
struct TagJump {
	HWND		hwndReferer;				//!< 参照元ウィンドウ
	CLogicPoint	point;						//!< ライン, カラム
};

//共有メモリ内構造体
//2004/06/21 タグジャンプ機能追加
//2005.04.03 MIK キーワード指定タグジャンプ
struct SShare_TagJump{
	//型
	typedef StaticVector<
		STagjumpKeyword,
		MAX_TAGJUMP_KEYWORD
	>					ATagJumpKeywords;

	//データ
	int					m_TagJumpNum;					//!< タグジャンプ情報の有効データ数
	int					m_TagJumpTop;					//!< スタックの一番上の位置
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< タグジャンプ情報
	ATagJumpKeywords	m_aTagJumpKeywords;
	BOOL				m_bTagJumpICase;				//!< 大文字小文字を同一視
	BOOL				m_bTagJumpPartialMatch;				//!< 文字列の途中にマッチ
};

class CTagJumpManager : private env::ShareDataClient {
public:
	//タグジャンプ関連	// 2004/06/21 novice タグジャンプ機能追加
	void PushTagJump(const TagJump * pTagJump);		//!< タグジャンプ情報の保存
	bool PopTagJump(TagJump *pTagJump);				//!< タグジャンプ情報の参照
};

#endif /* SAKURA_CTAGJUMPMANAGER_487A43FC_EB78_44CF_B1E4_6FD78EF1F35A_H_ */
