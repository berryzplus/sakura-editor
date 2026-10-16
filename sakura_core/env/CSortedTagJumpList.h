/*!	@file
	@brief タグジャンプリスト

	@author MIK
	@date 2005.3.31
*/
/*
	Copyright (C) 2005, MIK, genta
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CSORTEDTAGJUMPLIST_54B68DB2_20F2_4F62_BB2C_47629293620F_H_
#define SAKURA_CSORTEDTAGJUMPLIST_54B68DB2_20F2_4F62_BB2C_47629293620F_H_
#pragma once

#include "basis/primitive.h"
#include "util/design_template.h"

#include <list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#define MAX_TAG_STRING_LENGTH _MAX_PATH	//管理する文字列の最大長

/*!	@brief ダイレクトタグジャンプ用検索結果をソートして保持する．
	@author MIK
*/
class CSortedTagJumpList {
public:
	explicit CSortedTagJumpList(int max);
	~CSortedTagJumpList();

	int		AddBaseDir(std::wstring_view baseDir);

	BOOL AddParamA(
		std::string_view keyword,
		std::string_view filename,
		int no,
		ACHAR type,
		std::string_view note,
		int depth,
		const int baseDirId
	);

	BOOL GetParam(
		int index,
		std::span<WCHAR> filename,
		int* no,
		int* depth,
		std::span<WCHAR> baseDir
	);

	int		GetCount() const noexcept { return m_nCount; }
	void	Empty() noexcept;
	bool	IsOverflow() const noexcept { return m_bOverflow; }

	struct TagJumpInfo
	{
		std::wstring	keyword{};		//!< キーワード
		std::wstring	filename{};		//!< ファイル名
		int				no = 0;			//!< 行番号
		WCHAR			type = 0;		//!< 種類
		std::wstring	note{};			//!< 備考
		int				depth = 0;		//!< (さかのぼる)階層
		int				baseDirId = 0;	//!< ファイル名のベースディレクトリ
	};

	TagJumpInfo* GetPtr(int index);

	/*!	@brief 管理数の最大値を取得する

		@date 2005.04.22 genta 最大値を可変に
	*/
	int	GetCapacity() const noexcept { return m_MAX_TAGJUMPLIST; }

private:
	using BaseDirArr = std::vector<std::wstring>;
	using TagJumpInfoArr = std::list<TagJumpInfo>;

	TagJumpInfoArr	m_TagJumpArr{};			//!< タグジャンプ情報配列
	BaseDirArr		m_baseDirArr{};			//!< ベースディレクトリ情報
	int				m_nCount = 0;			//!< 個数
	bool			m_bOverflow = false;	//!< オーバーフロー
	
	//	2005.04.22 genta 最大値を可変に
	const int		m_MAX_TAGJUMPLIST;	//!< 管理する情報の最大数

	DISALLOW_COPY_AND_ASSIGN(CSortedTagJumpList);
};

#endif /* SAKURA_CSORTEDTAGJUMPLIST_54B68DB2_20F2_4F62_BB2C_47629293620F_H_ */
