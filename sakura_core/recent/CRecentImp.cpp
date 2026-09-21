/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentImp.h"

#include "CRecentCmd.h"
#include "CRecentCurDir.h"
#include "CRecentEditNode.h"
#include "CRecentExceptMru.h"
#include "CRecentExcludeFile.h"
#include "CRecentExcludeFolder.h"
#include "CRecentFile.h"
#include "CRecentFolder.h"
#include "CRecentGrepFile.h"
#include "CRecentGrepFolder.h"
#include "CRecentReplace.h"
#include "CRecentSearch.h"
#include "CRecentTagjumpKeyword.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	初期生成処理

	@note
	nCmpType = strcmp, stricmp のときに nCmpSize = 0 を指定すると、AppendItem 
	でのデータが文字列であると認識して strcpy をする。
	他の場合は memcpy で nItemSize 分をコピーする。
	
	pnViewCount = NULL にすると、擬似的に nViewCount == nArrayCount になる。
*/
template <class T>
bool CRecentImp<T>::Create(
	DataType*		pszItemArray,	//!< アイテム配列へのポインタ
	size_t			nTextMaxLength,	//!< 最大テキスト長(終端含む)
	int*			pnItemCount,	//!< アイテム個数へのポインタ
	bool*			pbItemFavorite,	//!< お気に入りへのポインタ(NULL許可)
	int				nArrayCount,	//!< 最大管理可能なアイテム数
	int*			pnViewCount		//!< 表示個数(NULL許可)
)
{
	//パラメータチェック
	if( nullptr == pszItemArray ) return false;
	if( nullptr == pnItemCount ) return false;
	if( nArrayCount <= 0 ) return false;
	if( pnViewCount && (*pnViewCount < 0 || nArrayCount < *pnViewCount) ) return false;

	//各パラメータ格納
	m_puUserItemData		= pszItemArray;
	m_nTextMaxLength		= nTextMaxLength;
	m_pnUserItemCount		= pnItemCount;
	m_pbUserItemFavorite	= pbItemFavorite;
	m_nArrayCount			= nArrayCount;
	m_pnUserViewCount		= pnViewCount;
	m_bCreate = true;

	//個別に操作されていたときのための対応
	UpdateView();

	return true;
}

/*
	終了処理
*/
template <class T>
void CRecentImp<T>::Terminate()
{
	m_bCreate = false;

	m_puUserItemData     = nullptr;
	m_pnUserItemCount    = nullptr;
	m_pnUserViewCount    = nullptr;
	m_pbUserItemFavorite = nullptr;

	m_nArrayCount  = 0;
}

/*
	初期化済みか調べる。
*/
template <class T>
bool CRecentImp<T>::IsAvailable() const
{
	if(!m_bCreate)return false;

	//データ破壊時のリカバリをやってみたりする
	const_cast<CRecentImp*>(this)->_Recovery(); 

	return true;
}

//! リカバリ
template <class T>
void CRecentImp<T>::_Recovery()
{
	if( *m_pnUserItemCount < 0             ) *m_pnUserItemCount = 0;
	if( *m_pnUserItemCount > m_nArrayCount ) *m_pnUserItemCount = m_nArrayCount;

	if( m_pnUserViewCount )
	{
		if( *m_pnUserViewCount < 0             ) *m_pnUserViewCount = 0;
		if( *m_pnUserViewCount > m_nArrayCount ) *m_pnUserViewCount = m_nArrayCount;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        お気に入り                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	お気に入り状態を設定する。

	true	設定
	false	解除
*/
template <class T>
bool CRecentImp<T>::SetFavorite( int nIndex, bool bFavorite )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( nullptr == m_pbUserItemFavorite ) return false;

	m_pbUserItemFavorite[nIndex] = bFavorite;

	return true;
}

/*
	すべてのお気に入り状態を解除する。
*/
template <class T>
void CRecentImp<T>::ResetAllFavorite()
{
	if( ! IsAvailable() ) return;

	for( int i = 0; i < *m_pnUserItemCount; i++ )
	{
		SetFavorite( i, false );
	}
}

/*
	お気に入り状態かどうか調べる。

	true	お気に入り
	false	通常
*/
template <class T>
bool CRecentImp<T>::IsFavorite( int nIndex ) const
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;
	if( nullptr == m_pbUserItemFavorite ) return false;

	return m_pbUserItemFavorite[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アイテム制御                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムを先頭に追加する。

	@note	すでに登録済みの場合は先頭に移動する。
	@note	いっぱいのときは最古のアイテムを削除する。
	@note	お気に入りは削除されない。
*/
template <class T>
bool CRecentImp<T>::AppendItemText( LPCWSTR pText )
{
	DataType data;
	if( !TextToDataType( &data, pText ) ){
		return false;
	}
	int findIndex = FindItem(&data);
	if( -1 != findIndex ){
		return false;
	}
	return AppendItem(&data);
}

template <class T>
bool CRecentImp<T>::EditItemText( int nIndex, LPCWSTR pText )
{
	DataType data;
	memcpy_raw( &data, GetItemPointer( nIndex ), sizeof(data) );
	if( !TextToDataType( &data, pText ) ){
		return false;
	}
	int findIndex = FindItem(&data);
	if( -1 != findIndex && nIndex != findIndex ){
		// 重複不可。ただし同じ場合は大文字小文字の変更かもしれないのでOK
		return false;
	}
	CopyItem(GetItemPointer(nIndex), &data);
	return true;
}

/*
	アイテムをゼロクリアする。
*/
template <class T>
void CRecentImp<T>::ZeroItem( int nIndex )
{
	if( ! IsAvailable() ) return;
	if( nIndex < 0 || nIndex >= m_nArrayCount ) return;

	memset_raw( GetItemPointer( nIndex ), 0, sizeof(DataType) );

	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nIndex] = false;

	return;
}

/*
	アイテムを削除する。
*/
template <class T>
bool CRecentImp<T>::DeleteItem( int nIndex )
{
	if( ! IsAvailable() ) return false;
	if( nIndex < 0 || nIndex >= *m_pnUserItemCount ) return false;

	ZeroItem( nIndex );

	//以降のアイテムを前に詰める。
	int i;
	for( i = nIndex; i < *m_pnUserItemCount - 1; i++ )
	{
		CopyItem( i + 1, i );
	}
	ZeroItem( i );

	*m_pnUserItemCount -= 1;

	return true;
}

/*
	お気に入り以外のアイテムを削除する。
*/
template <class T>
bool CRecentImp<T>::DeleteItemsNoFavorite()
{
	if( ! IsAvailable() ) return false;

	bool bDeleted = false;
	int i;
	for( i = *m_pnUserItemCount - 1; 0 <= i; i-- )
	{
		if( false == IsFavorite( i ) )
		{
			if( DeleteItem( i ) )
			{
				bDeleted = true;
			}
		}
	}

	return bDeleted;
}

/*
	すべてのアイテムを削除する。

	@note	ゼロクリアを可能とするため、すべてが対象になる。
*/
template <class T>
void CRecentImp<T>::DeleteAllItem()
{
	int	i;

	if( ! IsAvailable() ) return;

	for( i = 0; i < m_nArrayCount; i++ )
	{
		ZeroItem( i );
	}

	*m_pnUserItemCount = 0;

	return;
}

/*
	アイテムを移動する。
*/
template <class T>
bool CRecentImp<T>::MoveItem( int nSrcIndex, int nDstIndex )
{
	int	i;
	bool	bFavorite;

	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= *m_pnUserItemCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= *m_pnUserItemCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	DataType pri;

	//移動する情報を退避
	memcpy_raw( &pri, GetItemPointer( nSrcIndex ), sizeof(pri) );
	bFavorite = IsFavorite( nSrcIndex );

	if( nSrcIndex < nDstIndex )
	{
		for( i = nSrcIndex; i < nDstIndex; i++ )
		{
			CopyItem( i + 1, i );
		}
	}
	else
	{
		for( i = nSrcIndex; i > nDstIndex; i-- )
		{
			CopyItem( i - 1, i );
		}
	}

	//新しい位置に格納
	memcpy_raw( GetItemPointer( nDstIndex ), &pri, sizeof(pri) );
	SetFavorite( nDstIndex, bFavorite );

	return true;
}

template <class T>
bool CRecentImp<T>::CopyItem( int nSrcIndex, int nDstIndex )
{
	if( ! IsAvailable() ) return false;
	if( nSrcIndex < 0 || nSrcIndex >= m_nArrayCount ) return false;
	if( nDstIndex < 0 || nDstIndex >= m_nArrayCount ) return false;

	if( nSrcIndex == nDstIndex ) return true;

	memcpy_raw( GetItemPointer( nDstIndex ), GetItemPointer( nSrcIndex ), sizeof(DataType) );

	//(void)SetFavorite( nDstIndex, IsFavorite( nSrcIndex ) );
	//内部処理しないとだめ。
	if( m_pbUserItemFavorite ) m_pbUserItemFavorite[nDstIndex] = m_pbUserItemFavorite[nSrcIndex];

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アイテム取得                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムリストからもっとも古い｛お気に入り・通常｝のアイテムを探す。

	bFavorite=true	お気に入りの中から探す
	bFavorite=false	通常の中から探す
*/
template <class T>
int CRecentImp<T>::GetOldestItem( int nIndex, bool bFavorite )
{
	if( ! IsAvailable() ) return -1;
	if( nIndex >= *m_pnUserItemCount ) nIndex = *m_pnUserItemCount - 1;

	for( int i = nIndex; i >= 0; i-- )
	{
		if( IsFavorite( i ) == bFavorite ) return i;
	}

	return -1;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	管理されているアイテムのうちの表示個数を変更する。

	@note	お気に入りは可能な限り表示内に移動させる。
*/
template <class T>
bool CRecentImp<T>::ChangeViewCount( int nViewCount )
{
	int	i;
	int	nIndex;

	//範囲外ならエラー
	if( ! IsAvailable() ) return false;
	if( nViewCount < 0 || nViewCount > m_nArrayCount ) return false;

	//表示個数を更新する。
	if( m_pnUserViewCount )
	{
		*m_pnUserViewCount = nViewCount;
	}

	//範囲内にすべて収まっているので何もしなくてよい。
	if( nViewCount >= *m_pnUserItemCount ) return true;

	//最も古いお気に入りを探す。
	i = GetOldestItem( *m_pnUserItemCount - 1, true );
	if( -1 == i ) return true;	//ないので何もしないで終了

	//表示外アイテムを表示内に移動する。
	for( ; i >= nViewCount; i-- )
	{
		if( IsFavorite( i ) )
		{
			//カレント位置から上に通常アイテムを探す
			nIndex = GetOldestItem( i - 1, false );
			if( -1 == nIndex ) break;	//もう1個もない

			//見つかったアイテムをカレント位置に移動する
			MoveItem( nIndex, i );
		}
	}

	return true;
}

/*
	リストを更新する。
*/
template <class T>
bool CRecentImp<T>::UpdateView()
{
	int	nViewCount;

	//範囲外ならエラー
	if( ! IsAvailable() ) return false;

	if( m_pnUserViewCount ) nViewCount = *m_pnUserViewCount;
	else                    nViewCount = m_nArrayCount;

	return ChangeViewCount( nViewCount );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      インスタンス化                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
template class CRecentImp<SCmdString>;
template class CRecentImp<EditNode>;
template class CRecentImp<EditInfo>;
template class CRecentImp<SPathString>;
template class CRecentImp<SGrepFile>;
#ifndef __MINGW32__
template class CRecentImp<SMetaPath>;
template class CRecentImp<SGrepFolder>;
template class CRecentImp<SSearchString>;
template class CRecentImp<STagjumpKeyword>;
template class CRecentImp<SDirPath>;
template class CRecentImp<SReplaceString>;
#endif
