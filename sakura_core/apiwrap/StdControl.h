/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STDCONTROL_57A7282D_B9F0_4642_ABFF_48B6D715CCA7_H_
#define SAKURA_STDCONTROL_57A7282D_B9F0_4642_ABFF_48B6D715CCA7_H_
#pragma once

/*
2007.09.17 kobake
内部コードがWCHARなので、検索キーワードなどもWCHARで保持する。
そのため、検索ダイアログのコンボボックスなどに、WCHARを設定する場面が出てくる。
UNICODE版では問題無いが、ANSI版では設定の前にコード変換する必要がある。
呼び出し側で変換しても良いが、頻度が多いので、WCHARを直接受け取るAPIラップ関数を提供する。

また、SendMessageの直接呼び出しは、どうしてもWPARAM,LPARAMへの強制キャストが生じるため、
コンパイラの型チェックが働かず、wchar_t, charの混在するソースコードの中ではバグの温床になりやすい。
そういった意味でも、このファイル内のラップ関数を使うことを推奨する。
*/

#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include "mem/CNativeW.h"
#include "util/window.h"

namespace ApiWrap{

using namespace std::literals::string_view_literals;

/*!
 * @brief テキスト取得結果構造体
 */
struct SGetTextResult {
	bool success = false;	//!< テキストの取得に成功したか
	std::wstring text;		//!< 取得したテキスト

	//! デフォルトコンストラクタは失敗状態を表す
	SGetTextResult() = default;

	//! 成功状態を表すコンストラクタ
	explicit SGetTextResult(std::wstring&& result) noexcept
		: success(true)
		, text(std::move(result))
	{
	}

	/*!
	 * @brief 成功状態を返す変換演算子
	 *
	 * @retval true 取得成功。
	 * @note テキストが空でもtrue。
	 */
	explicit operator bool() const noexcept { return success; }

	/*!
	 * @brief 取得した文字列を返す変換演算子
	 *
	 * @note 失敗状態でも空文字列が返る。
	 */
	explicit operator const std::wstring& () const noexcept { return text; }
};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ウィンドウ共通                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool Wnd_SetText(HWND hWnd, std::wstring_view str);

	inline BOOL Wnd_SetText(HWND hwnd, const WCHAR* str)
	{
		return Wnd_SetText(hwnd, str ? std::wstring_view{ str } : L""sv);
	}

	/*!
		@brief Window テキストを設定する
		@param[in]  hwnd	ウィンドウハンドル
		@param[in]  str		ウィンドウテキスト
	*/
	inline BOOL Wnd_SetText(HWND hwnd, const CNativeW& str)
	{
		return Wnd_SetText(hwnd, std::wstring_view(str));
	}

	/*!
		@brief Window テキストを取得する
		@param[in]  hWnd	ウィンドウハンドル
		@param[out] strText	ウィンドウテキストを受け取る変数
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	bool Wnd_GetText( HWND hWnd, std::wstring& strText );

	bool Wnd_GetText(HWND hWnd, CNativeW& str);

/*!
 * @brief 指定したウインドウのテキストを取得する
 */
inline SGetTextResult GetWindowTextW(HWND hWnd)
{
	if (std::wstring buffer; Wnd_GetText(hWnd, buffer)) {
		return SGetTextResult(std::move(buffer));
	}
	return SGetTextResult();
}

/*!
 * @brief 指定したウインドウのテキストを変更する
 */
inline bool SetWindowTextW(HWND hWnd, std::wstring_view text)
{
	return Wnd_SetText(hWnd, text);
}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      コンボボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int Combo_AddString(HWND hWndCombo, std::wstring_view str);

	inline LRESULT Combo_GetLBText(HWND hwndCombo, int nIndex, WCHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_GETLBTEXT, nIndex, LPARAM(str) );
	}

	inline LRESULT Combo_GetText(HWND hwndCombo, WCHAR* str, int cchMax)
	{
		return ::GetWindowText( hwndCombo, str, cchMax );
	}

	inline int Combo_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_DELETESTRING, (WPARAM)index, 0L); }
	inline int Combo_FindStringExact(HWND hwndCtl, int indexStart, const WCHAR* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, LPARAM(lpszFind) ); }
	
	inline int Combo_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L); }
	inline int Combo_GetCurSel(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCURSEL, 0L, 0L); }
	inline int Combo_SetCurSel(HWND hwndCtl, size_t index) noexcept		{ return ComboBox_SetCurSel(hwndCtl, index); }
	inline LRESULT Combo_GetItemData(HWND hwndCtl, int index)			{ return ((LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0L)); }
	inline int Combo_SetItemData(HWND hwndCtl, int index, int data)		{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int Combo_SetItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int Combo_GetLBTextLen(HWND hwndCtl, size_t index)			{ return ComboBox_GetLBTextLen(hwndCtl, index); }
	inline int Combo_InsertString(HWND hwndCtl, size_t index, LPCWSTR lpsz) noexcept { return ComboBox_InsertString(hwndCtl, index, lpsz); }
	inline int Combo_LimitText(HWND hwndCtl, size_t cchLimit) noexcept	{ return ComboBox_LimitText(hwndCtl, cchLimit); }
	inline int Combo_ResetContent(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_RESETCONTENT, 0L, 0L); }
	inline int Combo_SetEditSel(HWND hwndCtl, int ichStart, int ichEnd)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEDITSEL, 0L, MAKELPARAM(ichStart, ichEnd)); }
	inline int Combo_SetExtendedUI(HWND hwndCtl, UINT flags)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEXTENDEDUI, (WPARAM)flags, 0L); }
	inline BOOL Combo_ShowDropdown(HWND hwndCtl, BOOL fShow)			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, CB_SHOWDROPDOWN, (WPARAM)fShow, 0L); }
	inline int Combo_SetDroppedWidth(HWND hwndCtl, int width)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETDROPPEDWIDTH, (WPARAM)width, 0L); }
	inline BOOL Combo_GetDroppedState(HWND hwndCtl)						{ return (BOOL)(DWORD)::SendMessage(hwndCtl, CB_GETDROPPEDSTATE, 0L, 0L ); }

	bool Combo_GetLBText(HWND hWndCombo, size_t index, std::wstring& str);
	bool Combo_GetLBText(HWND hWndCombo, size_t index, CNativeW& str);

	inline void Combo_GetEditSel( HWND hwndCombo, DWORD& dwSelStart, DWORD& dwSelEnd )
	{
		::SendMessage( hwndCombo, CB_GETEDITSEL, WPARAM( &dwSelStart ), LPARAM( &dwSelEnd ) );
	}
	inline void Combo_SHAutoComplete( HWND hwndCombo, DWORD dwFlags )
	{
		COMBOBOXINFO comboInfo;
		comboInfo.cbSize = sizeof(comboInfo);
		if (0 != GetComboBoxInfo(hwndCombo, &comboInfo))
		{
			SHAutoComplete(comboInfo.hwndItem, dwFlags);
		}
	}

/*!
 * @brief 指定したコンボボックスの入力文字数を制限する
 */
template<std::ranges::sized_range T>
inline int LimitCbText(HWND hWnd, const T& buffer)
{
	return Combo_LimitText(hWnd, std::size(buffer) - 1);
}

/*!
 * @brief コンボボックスに項目を追加する
 */
template<std::ranges::input_range R>
inline void AddCbItems(HWND hWnd, const R& items)
{
	for (const auto& item : items) {
		Combo_AddString(hWnd, item);
	}
}

/*!
 * @brief コンボボックスの項目テキストを取得する
 *
 * @note このメソッドが必要な実装には、おそらく問題がある。
 */
inline SGetTextResult GetCbItemText(HWND hWnd, size_t index)
{
	if (std::wstring buffer; Combo_GetLBText(hWnd, index, buffer)) {
		return SGetTextResult(std::move(buffer));
	}
	return SGetTextResult();
}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      リストボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool List_GetText(HWND hList, size_t index, std::wstring& strText);
	int List_GetText(HWND hwndList, size_t nIndex, std::span<WCHAR> buffer) noexcept;
	int List_GetText(HWND hwndList, size_t nIndex, WCHAR* pszText, size_t cchText) noexcept;
	int List_AddString(HWND hwndList, std::wstring_view str);
	inline int List_AddItemData(HWND hwndCtl, int data)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_AddItemData(HWND hwndCtl, void* data)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_DELETESTRING, (WPARAM)index, 0L); }
	inline int List_FindStringExact(HWND hwndCtl, int indexStart, WCHAR* lpszFind)	{ return (int)(DWORD)::SendMessage( hwndCtl, LB_FINDSTRINGEXACT, WPARAM(indexStart), LPARAM(lpszFind) ); }
	inline int List_GetCaretIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCARETINDEX, 0L, 0L); }
	inline int List_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCOUNT, 0L, 0L); }
	inline int List_GetCurSel(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCURSEL, 0L, 0L); }
	inline int List_GetTextLen(HWND hwndCtl, size_t index)				{ return ListBox_GetTextLen(hwndCtl, index); }
	inline int List_SetCurSel(HWND hwndCtl, int index)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETCURSEL, (WPARAM)index, 0L); }
	inline LRESULT List_GetItemData(HWND hwndCtl, int index)			{ return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, LB_GETITEMDATA, (WPARAM)index, 0L); }
	inline int List_SetItemData(HWND hwndCtl, int index, int data)		{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_SetItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_GetItemRect(HWND hwndCtl, int index, RECT* lprc)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETITEMRECT, (WPARAM)index, (LPARAM)lprc); }
	inline int List_GetTopIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETTOPINDEX, 0L, 0L); }
	inline int List_InsertItemData(HWND hwndCtl, int index, int data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertString(HWND hwndCtl, int index, const WCHAR* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, LPARAM(lpsz)); }
	inline BOOL List_ResetContent(HWND hwndCtl)							{ return (BOOL)(DWORD)::SendMessage(hwndCtl, LB_RESETCONTENT, 0L, 0L); }
	inline void List_SetHorizontalExtent(HWND hwndCtl, int cxExtent)	{ ::SendMessage(hwndCtl, LB_SETHORIZONTALEXTENT, (WPARAM)cxExtent, 0L); }
	inline int List_GetItemHeight(HWND hwndCtl, size_t index) noexcept { return ListBox_GetItemHeight(hwndCtl, index); }
	inline int List_SetItemHeight(HWND hwndCtl, size_t index, int cy) noexcept { return ListBox_SetItemHeight(hwndCtl, index, cy); }
	inline int List_SetTopIndex(HWND hwndCtl, int indexTop)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETTOPINDEX, (WPARAM)indexTop, 0L); }

/*!
 * @brief リストボックスに項目を追加する
 */
template<std::ranges::input_range R>
inline void AddLbItems(HWND hWnd, const R& items)
{
	for (const auto& item : items) {
		ApiWrap::List_AddString(hWnd, item);
	}
}

/*!
 * @brief リストボックスの項目テキストを取得する
 *
 * @note このメソッドが必要な実装には、おそらく問題がある。
 */
inline SGetTextResult GetLbItemText(HWND hWnd, size_t index)
{
	if (std::wstring buffer; List_GetText(hWnd, static_cast<int>(index), buffer)) {
		return SGetTextResult(std::move(buffer));
	}
	return SGetTextResult();
}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      エディット コントロール                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline void EditCtl_LimitText(HWND hwndCtl, size_t cchLimit) noexcept	{ Edit_LimitText(hwndCtl, cchLimit); }
	inline void EditCtl_SetSel(HWND hwndCtl, int ichStart, int ichEnd)	{ ::SendMessage(hwndCtl, EM_SETSEL, ichStart, ichEnd); }

	inline void EditCtl_ReplaceSel(HWND hwndCtl, const WCHAR* lpsz)		{ ::SendMessage(hwndCtl, EM_REPLACESEL, 0, LPARAM(lpsz)); }

/*!
 * @brief 指定したエディットコントロールの入力文字数を制限する
 */
template<std::ranges::sized_range T>
inline void LimitEditText(HWND hWnd, const T& buffer)
{
	EditCtl_LimitText(hWnd, std::size(buffer) - 1);
}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ボタン コントロール                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline int BtnCtl_GetCheck(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, BM_GETCHECK, 0L, 0L); }
	inline void BtnCtl_SetCheck(HWND hwndCtl, int check)				{ ::SendMessage(hwndCtl, BM_SETCHECK, (WPARAM)check, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      スタティック コントロール              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline HICON StCtl_SetIcon(HWND hwndCtl, HICON hIcon)				{ return (HICON)(UINT_PTR)::SendMessage(hwndCtl, STM_SETICON, (WPARAM)hIcon, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      スタティック コントロール              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline void TrackBarCtl_SetRange(HWND hwndCtl, BOOL bRedraw, WORD minimum, WORD maximum)
	{
		::SendMessage(hwndCtl, TBM_SETRANGE, (WPARAM)bRedraw, (LPARAM)MAKELONG(minimum, maximum));
	}
	inline void TrackBarCtl_SetPos(HWND hwndCtl, BOOL bRedraw, LPARAM pos)
	{
		::SendMessage(hwndCtl, TBM_SETPOS, (WPARAM)bRedraw, pos);
	}
	inline LRESULT TrackBarCtl_GetPos(HWND hwndCtl)
	{
		return ::SendMessage(hwndCtl, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ダイアログ内                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool DlgItem_SetText(HWND hWndDlg, int nIDDlgItem, std::wstring_view str);
	bool DlgItem_GetText(HWND hWndDlg, int nIDDlgItem, std::wstring& buffer);
	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, WCHAR* pszText, size_t nMaxCount) noexcept;

/*!
 * @brief ダイアログボックス項目のテキストを取得する
 */
inline SGetTextResult GetDlgItemTextW(HWND hDlg, int nIdDlgItem)
{
	if (std::wstring buffer; DlgItem_GetText(hDlg, nIdDlgItem, buffer)) {
		return SGetTextResult(std::move(buffer));
	}
	return SGetTextResult();
}

/*!
 * @brief 指定したウインドウのテキストを変更する
 */
inline bool SetDlgItemTextW(HWND hDlg, int nIdDlgItem, std::wstring_view text)
{
	return DlgItem_SetText(hDlg, nIdDlgItem, text);
}

inline void CheckDlgButton(HWND hDlg, int nIDButton, bool bCheck = true)
{
	CheckDlgButtonBool(hDlg, nIDButton, bCheck);
}

inline bool IsDlgButtonChecked(HWND hDlg, int nIDButton)
{
	return IsDlgButtonCheckedBool(hDlg, nIDButton);
}

inline bool EnableDlgItem(HWND hDlg, int nIDDlgItem, bool enable = true)
{
	return DlgItem_Enable(hDlg, nIDDlgItem, enable);
}

inline bool ShowDlgItem(HWND hDlg, int nIDDlgItem, bool show = true)
{
	const auto hWndCtl = ::GetDlgItem(hDlg, nIDDlgItem);
	if (!hWndCtl) return false;
	const auto nCmdShow = show ? SW_SHOW : SW_HIDE;
	return ::ShowWindow(hWndCtl, nCmdShow);
}

	bool TreeView_GetItemTextVector(HWND hwndTree, TVITEM& item, std::vector<WCHAR>& vecStr);
	void TreeView_ExpandAll( HWND, bool, int nMaxDepth = 100 );
}

#endif /* SAKURA_STDCONTROL_57A7282D_B9F0_4642_ABFF_48B6D715CCA7_H_ */
