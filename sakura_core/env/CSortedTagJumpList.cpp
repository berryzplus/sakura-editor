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

#include "StdAfx.h"
#include "env/CSortedTagJumpList.h"

#include "util/string_ex.h"
#include "util/tchar_convert.h"

#include <ranges>

/*!
	@date 2005.04.23 genta 管理数の最大値を指定する引数追加
*/
CSortedTagJumpList::CSortedTagJumpList(int max)
	: m_MAX_TAGJUMPLIST( max )
{
	// id==0 を 空文字列にする
	m_baseDirArr.emplace_back(L"");
}

CSortedTagJumpList::~CSortedTagJumpList()
{
	Empty();
}

/*
	リストをすべて解放する。
*/
void CSortedTagJumpList::Empty() noexcept
{
	m_TagJumpArr.clear();
	m_nCount = 0;
	m_bOverflow = false;
	m_baseDirArr.clear();
	m_baseDirArr.emplace_back(L"");
}

/*
	基準フォルダーを登録し、基準フォルダーIDを取得
	@date 2010.07.23 Moca 新規追加
*/
int CSortedTagJumpList::AddBaseDir(std::wstring_view baseDir)
{
	m_baseDirArr.emplace_back(baseDir);
	return int(m_baseDirArr.size() - 1);
}

/*
	アイテムをソートされた状態でリストに追加する。
	アイテムが最大数を超える場合は、超えるアイテムを削除する。
	文字列はコピーを作成するので、呼び出し側は文字列のアドレス先を保持する必要はない。
	
	@param[in] keyword	キーワード
	@param[in] filename	ファイル名
	@param[in] no		行番号
	@param[in] type		種類
	@param[in] note		備考
	@param[in] depth	(さかのぼる)階層
	@param[in] baseDirId	基準フォルダーID。0で空文字列指定 (AddBaseDirの戻り値)
	@retval TRUE  追加した
	@retval FALSE 追加失敗
	@date 2010.07.23 Moca baseDirId 追加
*/
BOOL CSortedTagJumpList::AddParamA(
	std::string_view keyword,
	std::string_view filename,
	int no,
	ACHAR type,
	std::string_view note,
	int depth,
	int baseDirId
)
{
	const auto wkeyword = cxx::to_wstring(keyword);

	auto found = std::ranges::find_if(m_TagJumpArr, [wkeyword](const TagJumpInfo& p) {
		return p.keyword.compare(wkeyword) > 0;
	});

	try
	{
		//アイテムを作成する。
		const auto& inserted = m_TagJumpArr.emplace(found,
			cxx::to_wstring(keyword),
			cxx::to_wstring(filename),
			no,
			type,
			cxx::to_wstring(note),
			depth,
			baseDirId
		);

		//文字列長ガード
		if (MAX_TAG_STRING_LENGTH <= inserted->keyword.length()) inserted->keyword.resize(MAX_TAG_STRING_LENGTH - 1);
		if (MAX_TAG_STRING_LENGTH <= inserted->filename.length()) inserted->filename.resize(MAX_TAG_STRING_LENGTH - 1);
		if (MAX_TAG_STRING_LENGTH <= inserted->note.length()) inserted->note.resize(MAX_TAG_STRING_LENGTH - 1);
	}
	catch (const std::bad_alloc&)
	{
		return FALSE;
	}

	//アイテムをリストの適当な位置に追加する。
	m_nCount++;

	//最大数を超えたら最後のアイテムを削除する。
	if (m_MAX_TAGJUMPLIST < m_nCount)
	{
		m_TagJumpArr.resize(m_MAX_TAGJUMPLIST);
		m_nCount--;
		m_bOverflow = true;
	}

	return TRUE;
}

/*
	指定の情報を取得する。

	@param[out] filename	ファイル名
	@param[out] no			行番号
	@param[out] depth		(さかのぼる)階層
	@param[out] baseDir		ファイル名の基準フォルダー
	@return 処理結果
 */
BOOL CSortedTagJumpList::GetParam(
	int index,
	std::span<WCHAR> filename,
	int* no,
	int* depth,
	std::span<WCHAR> baseDir
)
{
	filename[0] = L'\0';
	if( no       ) *no    = 0;
	if( depth    ) *depth = 0;
	baseDir[0] = L'\0';

	if (const auto p = GetPtr(index))
	{
		wcscpy_s( filename, p->filename );

		if( no       ) *no    = p->no;
		if( depth    ) *depth = p->depth;
		if( 0 <= p->baseDirId &&
			p->baseDirId < static_cast<int>(m_baseDirArr.size()))
		{
			wcscpy_s( baseDir, m_baseDirArr[p->baseDirId] );
		}
		return TRUE;
	}
	return FALSE;
}

/*
	指定の情報を構造体ポインタで取得する。
	取得した情報は参照なので解放してはならない。

	@param[in] index 要素番号
	@return タグジャンプ情報
*/
CSortedTagJumpList::TagJumpInfo* CSortedTagJumpList::GetPtr(int index)
{
	int i = 0;

	for (auto it = m_TagJumpArr.begin(); it != m_TagJumpArr.end(); ++it)
	{
		if (index == i) return std::to_address(it);
		++i;
	}

	return nullptr;
}
