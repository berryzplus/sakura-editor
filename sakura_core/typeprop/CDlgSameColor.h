/*!	@file
	@brief 文字色／背景色統一ダイアログ

	@author ryoji
	@date 2006/04/26 作成
*/
/*
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGSAMECOLOR_181C0F46_A420_4A62_A543_FE2B88C20FBE_H_
#define SAKURA_CDLGSAMECOLOR_181C0F46_A420_4A62_A543_FE2B88C20FBE_H_
#pragma once

#include "dlg/CDialog.h"
#include "window/CWnd.h"

struct STypeConfig;

/*!	@brief 文字色／背景色統一ダイアログ

	タイプ別設定のカラー設定で，文字色／背景色統一の対象色を指定するために補助的に
	使用されるダイアログボックス
*/
class CDlgSameColor final : public CDialog
{
private:
	using Base = CDialog;
	using Me = CDlgSameColor;

public:
	struct ColorStatic final : public TCustomizedCtrl<CDlgSameColor> {
		explicit ColorStatic(CDlgSameColor& cDlgSameColor) : TCustomizedCtrl(cDlgSameColor) {}

		LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	};

	struct ColorList final : public CCustomizedWnd {
		LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	};

	CDlgSameColor();
	~CDlgSameColor() override;

	int DoModal( HINSTANCE hInstance, HWND hwndParent, WORD wID, STypeConfig* pTypes, COLORREF cr );		//!< モーダルダイアログの表示

protected:

	LPVOID GetHelpIdTable( void ) override;
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;	//! ダイアログのメッセージ処理
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam ) override;			//!< WM_INITDIALOG 処理
	BOOL OnBnClicked( int wID ) override;							//!< BN_CLICKED 処理
	BOOL OnDrawItem( WPARAM wParam, LPARAM lParam ) override;	//!< WM_DRAWITEM 処理
	BOOL OnSelChangeListColors( HWND hwndCtl );					//!< 色選択リストの LBN_SELCHANGE 処理

	WORD m_wID = 0;			//!< タイプ別設定ダイアログ（親ダイアログ）で押されたボタンID
	STypeConfig* m_pTypes = nullptr;	//!< タイプ別設定データ
	COLORREF m_cr = 0;		//!< 指定色

	ColorStatic		m_ColorStatic{ *this };
	ColorList		m_ColorList;
};

#endif /* SAKURA_CDLGSAMECOLOR_181C0F46_A420_4A62_A543_FE2B88C20FBE_H_ */
