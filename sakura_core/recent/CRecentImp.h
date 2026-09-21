/*! @file */
// 各CRecent実装クラスのベースクラス
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTIMP_B18E6196_5684_44E4_91E0_ADB1542BF7E1_H_
#define SAKURA_CRECENTIMP_B18E6196_5684_44E4_91E0_ADB1542BF7E1_H_
#pragma once

#include "recent/CRecent.h"

#include <type_traits>

template <typename DATA_TYPE, typename TEXT_TYPE = DATA_TYPE>
class CRecentImp : public CRecent{
	using Me = CRecentImp<DATA_TYPE, TEXT_TYPE>;

	using DataType = DATA_TYPE;
	using TextType = TEXT_TYPE;

public:
	CRecentImp(){ Terminate(); }

	CRecentImp(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	CRecentImp(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;

	~CRecentImp() override { Terminate(); }

protected:
	//生成
	bool Create(
		DataType*		pszItemArray,	//!< アイテム配列へのポインタ
		size_t			nTextMaxLength,	//!< 最大テキスト長(終端含む)
		int*			pnItemCount,	//!< アイテム個数へのポインタ
		bool*			pbItemFavorite,	//!< お気に入りへのポインタ(NULL許可)
		int				nArrayCount,	//!< 最大管理可能なアイテム数
		int*			pnViewCount		//!< 表示個数(NULL許可)
	);

public:
	void Terminate() override;
	bool IsAvailable() const;
	void _Recovery();

	//更新
	bool ChangeViewCount( int nViewCount );	//表示数の変更
	bool UpdateView() override;

	//プロパティ取得系
	int GetArrayCount() const override { return m_nArrayCount; }	//最大要素数
	int GetItemCount() const override { return ( IsAvailable() ? *m_pnUserItemCount : 0); }	//登録アイテム数
	int GetViewCount() const override { return ( IsAvailable() ? (m_pnUserViewCount ? *m_pnUserViewCount : m_nArrayCount) : 0); }	//表示数
	size_t GetTextMaxLength() const override { return m_nTextMaxLength; }

	//お気に入り制御系
	bool SetFavorite( int nIndex, bool bFavorite = true) override;	//お気に入りに設定
	bool ResetFavorite( int nIndex ) { return SetFavorite( nIndex, false ); }	//お気に入りを解除
	void ResetAllFavorite();			//お気に入りをすべて解除
	bool IsFavorite( int nIndex ) const override;			//お気に入りか調べる

	//アイテム制御
	template <class A>
		requires std::convertible_to<A, const DataType*> || basis::NullTerminatedStringConstructible<A, WCHAR>
	bool AppendItem(const A& itemData)
	{
		if (!IsAvailable() || !ValidateItem(itemData)) return false;

		int nIndex = FindItem(itemData);
		if (nIndex >= 0) {
			CopyItem(GetItemPointer(nIndex), itemData);
			MoveItem(nIndex, 0);
		}
		else {
			if (m_nArrayCount <= *m_pnUserItemCount) {
				nIndex = GetOldestItem(*m_pnUserItemCount - 1, false);
				if (-1 == nIndex) return false;
				DeleteItem(nIndex);
			}

			for (int i = *m_pnUserItemCount; i > 0; --i) {
				CopyItem(i - 1, i);
			}
			CopyItem(GetItemPointer(0), itemData);
			if (m_pbUserItemFavorite) m_pbUserItemFavorite[0] = false;
			*m_pnUserItemCount += 1;
		}

		if (m_pnUserViewCount) {
			ChangeViewCount(*m_pnUserViewCount);
		}
		return true;
	}
	bool AppendItemText( LPCWSTR pszText ) override;
	bool EditItemText( int nIndex, LPCWSTR pszText ) override;
	bool DeleteItem( int nIndex ) override;				//アイテムをクリア
	template <class A>
		requires std::convertible_to<A, const DataType*> || basis::NullTerminatedStringConstructible<A, WCHAR>
	bool DeleteItem(const A& itemData)
	{
		return DeleteItem(FindItem(itemData));
	}
	bool DeleteItemsNoFavorite() override;			//お気に入り以外のアイテムをクリア
	void DeleteAllItem() override;					//アイテムをすべてクリア

	//アイテム取得
protected:
	const DataType* GetItemPointer(int nIndex) const {
		if (!IsAvailable()) throw std::logic_error("CRecentImp: not available");
		if (nIndex < 0 || m_nArrayCount <= nIndex) throw std::out_of_range("out of range");
		return &m_puUserItemData[nIndex];
	}
	DataType* GetItemPointer(int nIndex) {
		if (!IsAvailable()) throw std::logic_error("CRecentImp: not available");
		if (nIndex < 0 || m_nArrayCount <= nIndex) throw std::out_of_range("out of range");
		return &m_puUserItemData[nIndex];
	}

public:
	auto& GetItem(int nIndex) const { return *GetItemPointer(nIndex); }
	auto& GetItem(int nIndex)		{ return *GetItemPointer(nIndex); }

protected:
	virtual TextType& GetItemString(int nIndex)
	{
		if constexpr (std::is_same_v<DataType, TextType>) {
			return GetItem(nIndex);
		}
		else {
			throw std::logic_error("CRecentImp: GetItemString must be overridden");
		}
	}
	virtual const TextType& GetItemString(int nIndex) const
	{
		if constexpr (std::is_same_v<DataType, TextType>) {
			return GetItem(nIndex);
		}
		else {
			throw std::logic_error("CRecentImp: GetItemString must be overridden");
		}
	}

public:
	const WCHAR* GetItemText(int nIndex) const override
	{
		return GetItemString(nIndex);
	}

	template <class A>
		requires std::convertible_to<A, const DataType*> || basis::NullTerminatedStringConstructible<A, WCHAR>
	int FindItem(const A& itemData) const
	{
		if (!IsAvailable() || !ValidateItem(itemData)) return -1;

		for (int i = 0; i < *m_pnUserItemCount; ++i) {
			if (0 == CompareItem(GetItemPointer(i), itemData)) return i;
		}
		return -1;
	}
	bool MoveItem( int nSrcIndex, int nDstIndex );	//アイテムを移動

	//オーバーライド用インターフェース
	virtual int CompareItemData(const DataType* lhs, const DataType* rhs) const
	{
		if constexpr (requires { lhs->compare(*rhs); }) {
			return lhs->compare(*rhs);
		}
		else {
			return 0;
		}
	}

	template <class A>
		requires std::convertible_to<A, const DataType*> || basis::NullTerminatedStringConstructible<A, WCHAR>
	int CompareItem(const DataType* lhs, const A& rhs) const
	{
		if constexpr (basis::NullTerminatedStringConstructible<A, WCHAR>) {
			return lhs->compare(rhs);
		}
		else {
			return CompareItemData(lhs, rhs);
		}
	}

	template <class A>
		requires std::convertible_to<A, const DataType*> || basis::NullTerminatedStringConstructible<A, WCHAR>
	void CopyItem(DataType* dst, const A& src) const
	{
		if constexpr (basis::NullTerminatedStringConstructible<A, WCHAR>) {
			wcscpy_s(*dst, src);
		}
		else {
			*dst = *src;
		}
	}

	virtual bool TextToDataType(
		 DataType* dst [[maybe_unused]],
		 LPCWSTR pszText [[maybe_unused]]
	) const
	{
		if constexpr (basis::NullTerminatedStringConstructible<DataType, WCHAR>) {
			if (!ValidateItem(pszText)) {
				return false;
			}
			CopyItem(dst, pszText);
			return true;
		}
		else {
			return false;
		}
	}

	template <class A>
		requires std::convertible_to<A, const DataType*> || basis::NullTerminatedStringConstructible<A, WCHAR>
	bool ValidateItem(const A& value) const
	{
		if constexpr (basis::NullTerminatedStringConstructible<A, WCHAR>) {
			const auto text = cxx::NullTerminatedString{ value };
			return text.c_str() && text.str().length() < GetTextMaxLength();
		}
		else {
			return nullptr != value;
		}
	}

	int FindItemByText(LPCWSTR pszText) const override
	{
		int n = GetItemCount();
		for(int i=0;i<n;i++){
			if(wcscmp(GetItemText(i),pszText)==0)return i;
		}
		return -1;
	}

	// 共有メモリアクセス
	DLLSHAREDATA*	GetShareData()
	{
		return &GetDllShareData();
	}

	//実装補助
private:
	void   ZeroItem( int nIndex );	//アイテムをゼロクリアする
	int    GetOldestItem( int nIndex, bool bFavorite );	//最古のアイテムを探す
	bool   CopyItem( int nSrcIndex, int nDstIndex );

private:
	//内部フラグ
	bool		m_bCreate;				//!< Create済みか

	//外部参照
	DataType*	m_puUserItemData;		//!< アイテム配列へのポインタ
	int*		m_pnUserItemCount;		//!< アイテム個数へのポインタ
	bool*		m_pbUserItemFavorite;	//!< お気に入りへのポインタ (NULL許可)
	int			m_nArrayCount;			//!< 最大管理可能なアイテム数
	int*		m_pnUserViewCount;		//!< 表示個数 (NULL許可)
	size_t		m_nTextMaxLength;		//!< 最大テキスト長(終端含む)
};

#endif /* SAKURA_CRECENTIMP_B18E6196_5684_44E4_91E0_ADB1542BF7E1_H_ */
