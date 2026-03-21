/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "apiwrap/StdControl.h"

#include "apiwrap/StdApi.h"

namespace ApiWrap{

bool Wnd_SetText(HWND hWnd, std::wstring_view str)
{
	std::wstring buffer;
	if (const auto p = std::data(str); str.empty() || p[str.size()]) {
		buffer = str;
		str = buffer;
	}
	return ::SetWindowTextW(hWnd, std::data(str));
}

	/*!
		@brief Window テキストを取得する
		@param[in]  hWnd	ウィンドウハンドル
		@param[out] buffer	ウィンドウテキストを受け取る変数
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	bool Wnd_GetText( HWND hWnd, std::wstring& strText )
	{
		// バッファをクリアしておく
		strText.clear();

		// GetWindowTextLength() はウィンドウテキスト取得に必要なバッファサイズを返す。
		// 条件によっては必要なサイズより大きな値を返すことがある模様
		// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-getwindowtextlengthw
		const int cchRequired = ::GetWindowTextLength( hWnd );
		if( cchRequired < 0 ){
			// ドキュメントには失敗した場合、あるいはテキストが空の場合には 0 を返すとある。
			// 0 の場合はエラーかどうか判断できないのでテキストの取得処理を続行する。
			// 仕様上は負の場合はありえないが、念の為エラーチェックしておく。
			return false;
		}else if( cchRequired == 0 ){
			// GetWindowTextLength はエラーの場合、またはテキストが空の場合は 0 を返す
			if( GetLastError() != 0 ){
				return false;
			}
			return true;
		}

		// ウィンドウテキストを取得するのに必要なバッファを確保する
		strText.resize( cchRequired + 1 );

		// GetWindowText() はコピーした文字数を返す。
		// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
		const int actualCopied = ::GetWindowText( hWnd, strText.data(), (int)strText.capacity() );
		if( actualCopied < 0 ){
			// 仕様上は負の場合はありえないが、念の為エラーチェックしておく。
			return false;
		}
		else if( actualCopied == 0 ){
			// GetWindowText はエラーの場合、またはテキストが空の場合は 0 を返す
			if( GetLastError() != 0 ){
				return false;
			}
		}
		else if( (int)strText.capacity() <= actualCopied ){
			// GetWindowText() の仕様上はありえないはず
			return false;
		}

		// データサイズを反映する
		strText.assign( strText.data(), actualCopied );

		return true;
	}

	/*!
	 * @brief Window テキストを取得する
	 * @param[in]  hwnd	ウィンドウハンドル
	 * @param[out] str		ウィンドウテキスト
	 * @retval true		成功した
	 * @retval false	失敗した
	 */
	bool Wnd_GetText(HWND hWnd, CNativeW& str)
	{
		// バッファをクリアしておく
		str.Clear();

		std::wstring buffer;
		if (!Wnd_GetText(hWnd, buffer)) {
			return false;
		}

		str.SetString(std::data(buffer), std::size(buffer));

		return true;
	}

int List_AddString(HWND hWndList, std::wstring_view str)
{
	std::wstring buffer;
	if (const auto p = std::data(str); str.empty() || p[str.size()]) {
		buffer = str;
		str = buffer;
	}
	return ListBox_AddString(hWndList, std::data(str));
}

	/*!
		@brief リストアイテムのテキストを取得する
		@param[in]  hList		リストコントロールのウインドウハンドル
		@param[in]  index		リストアイテムのインデックス
		@param[out] buffer		アイテムテキストを受け取る変数
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	bool List_GetText(HWND hList, size_t index, std::wstring& buffer)
	{
		// バッファをクリアしておく
		buffer.clear();

		const auto cchRequired = List_GetTextLen(hList, index);
		if (cchRequired <= 0) {
			// LB_ERR(-1)とその他のエラーは区別しない
			return false;
		}

		// アイテムテキストを設定するのに必要なバッファを確保する
		buffer.resize(cchRequired + 1);

		const auto actualCopied = List_GetText(hList, index, std::span<WCHAR>{ buffer });
		if (actualCopied < 0) {
			buffer.clear();
			return false;
		}

		buffer.resize(actualCopied);

		return true;
	}

	int List_GetText(HWND hwndList, size_t nIndex, std::span<WCHAR> buffer) noexcept
	{
		const auto nCount = List_GetTextLen(hwndList, nIndex);
		if (nCount < 0) {
			return LB_ERR;
		}
		if (std::ssize(buffer) <= nCount) {
			return LB_ERRSPACE;
		}
		return ListBox_GetText(hwndList, nIndex, std::data(buffer));
	}

	int List_GetText(HWND hwndList, size_t nIndex, WCHAR* pszText, size_t cchText) noexcept
	{
		return List_GetText(hwndList, nIndex, std::span(pszText, cchText));
	}

int Combo_AddString(HWND hWndCombo, std::wstring_view str)
{
	std::wstring buffer;
	if (const auto p = std::data(str); str.empty() || p[str.size()]) {
		buffer = str;
		str = buffer;
	}
	// CB_ADDSTRING は失敗の時、負の値(CB_ERR)を返す。
	// 成功した場合 0 ベースのインデックスを返す。
	return ComboBox_AddString(hWndCombo, std::data(str));
}

bool Combo_GetLBText(HWND hWndCombo, size_t index, std::wstring& buffer)
{
	// バッファをクリアしておく
	buffer.clear();

	// 文字列長を取得する、取得できなければエラー
	const auto length = Combo_GetLBTextLen(hWndCombo, index);
	if (length < 0) {
		return false;
	}

	// 必要なメモリを確保する
	buffer.resize(length + 1);

	// アイテムテキストを取得する
	const auto actualCount = Combo_GetLBText(hWndCombo, index, std::data(buffer));
	if (actualCount < 0) {
		buffer.clear();
		return false;
	}

	buffer.resize(actualCount);

	return true;
}

bool Combo_GetLBText(HWND hWndCombo, size_t index, CNativeW& str)
{
	// バッファをクリアしておく
	str.Clear();

	std::wstring buffer;
	if (!Combo_GetLBText(hWndCombo, index, buffer)) {
		return false;
	}

	str.SetString(std::data(buffer), std::size(buffer));

	return true;
}

bool DlgItem_SetText(HWND hWndDlg, int nIDDlgItem, std::wstring_view str)
{
	std::wstring buffer;
	if (const auto p = std::data(str); str.empty() || p[str.size()]) {
		buffer = str;
		str = buffer;
	}
	return ::SetDlgItemTextW(hWndDlg, nIDDlgItem, std::data(str));
}

	/*!
		@brief ダイアログアイテムのテキストを取得する
		@param[in]  hDlg		ウィンドウハンドル
		@param[in]  nIDDlgItem	ダイアログアイテムのID
		@param[out] buffer		アイテムテキストを受け取る変数
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	bool DlgItem_GetText(HWND hWndDlg, int nIDDlgItem, std::wstring& buffer)
	{
		// バッファをクリアしておく
		buffer.clear();

		// アイテムのハンドルを取得する
		const auto hWndCtl = ::GetDlgItem(hWndDlg, nIDDlgItem);
		if (!hWndCtl) return false;

		const auto cchRequired = ::GetWindowTextLengthW(hWndCtl);
		if (cchRequired < 0) return false;

		// ウィンドウテキストを取得するのに必要なバッファを確保する
		buffer.resize(cchRequired);

		// GetWindowText() はコピーした文字数を返す。
		const auto actualCopied = ::GetDlgItemTextW(hWndDlg, nIDDlgItem, buffer.data(), cchRequired + 1);
		if (!actualCopied) {
			buffer.clear();
			return false;
		}

		// データサイズを反映する
		buffer.resize(actualCopied);

		return true;
	}

	UINT DlgItem_GetText(HWND hWndDlg, int nIDDlgItem, WCHAR* pszText, size_t nMaxCount) noexcept
	{
		const auto hWndCtl = ::GetDlgItem(hWndDlg, nIDDlgItem);
		if (!hWndCtl) return false;

		const auto cchRequired = ::GetWindowTextLengthW(hWndCtl);
		if (cchRequired < 0) return false;

		return ::GetDlgItemTextW(hWndDlg, nIDDlgItem, pszText, int(nMaxCount));
	}

	bool TreeView_GetItemTextVector(HWND hwndTree, TVITEM& item, std::vector<WCHAR>& vecStr)
	{
		BOOL ret = FALSE;
		int nBufferSize = 64;
		while( FALSE == ret ){
			nBufferSize *= 2;
			if( 0x10000 < nBufferSize ){
				break;
			}
			vecStr.resize(nBufferSize);
			item.pszText = &vecStr[0];
			item.cchTextMax = (int)vecStr.size();
			ret = TreeView_GetItem(hwndTree, &item);
		}
		return FALSE != ret;
	}

	// TreeView 全開･全閉
	void TreeView_ExpandAll(HWND hwndTree, bool bExpand, int nMaxDepth)
	{
		HTREEITEM	htiCur;
		HTREEITEM	htiItem;
		HTREEITEM	htiNext;

		::SendMessageAny(hwndTree, WM_SETREDRAW, (WPARAM)FALSE, 0);

		htiCur = htiItem = TreeView_GetSelection( hwndTree );
		if (!bExpand && htiCur != nullptr) {
			// 閉じる時はトップに変更
			for (htiNext = htiCur; htiNext !=  nullptr; ) {
				htiItem = htiNext;
				htiNext = TreeView_GetParent( hwndTree, htiItem );
			}
			if (htiCur != htiItem) {
				htiCur = htiItem;
				TreeView_SelectItem( hwndTree, htiCur );
			}
		}

		std::vector<HTREEITEM> tree;
		HTREEITEM item = TreeView_GetRoot(hwndTree);
		while( 0 < tree.size() || item != nullptr ){
			while(item != nullptr && (int)tree.size() < nMaxDepth ){
				// 先に展開してからGetChildしないと、ファイルツリーのサブアイテムが展開されない
				TreeView_Expand(hwndTree, item, bExpand ? TVE_EXPAND : TVE_COLLAPSE);
				tree.push_back(item);
				item = TreeView_GetChild(hwndTree, item);
			}
			item = tree.back();
			tree.pop_back();
			item = TreeView_GetNextSibling(hwndTree, item);
		}

		// 選択位置を戻す
		if (htiCur == nullptr) {
			if (bExpand ) {
				htiItem = TreeView_GetRoot( hwndTree );
				TreeView_SelectSetFirstVisible( hwndTree, htiItem );
			}
			TreeView_SelectItem( hwndTree, nullptr );
		}
		else {
			TreeView_SelectSetFirstVisible( hwndTree, htiCur );
		}

		::SendMessageAny(hwndTree, WM_SETREDRAW, (WPARAM)TRUE, 0);
	}
}
