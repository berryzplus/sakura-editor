/*!	@file
	@brief Dialog Box基底クラスヘッダーファイル

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Uchi
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDIALOG_17C8C15C_881C_4C1F_B953_CB11FCC8B70B_H_
#define SAKURA_CDIALOG_17C8C15C_881C_4C1F_B953_CB11FCC8B70B_H_
#pragma once

#include "window/CWnd.h"

#include "sakura_rc.h"

struct DLLSHAREDATA;
class CRecent;

enum EAnchorStyle
{
	ANCHOR_NONE              = 0,
	ANCHOR_LEFT              = 1,
	ANCHOR_RIGHT             = 2,
	ANCHOR_LEFT_RIGHT        = 3,
	ANCHOR_TOP               = 4,
	ANCHOR_TOP_LEFT          = 5,
	ANCHOR_TOP_RIGHT         = 6,
	ANCHOR_TOP_LEFT_RIGHT    = 7,
	ANCHOR_BOTTOM            = 8,
	ANCHOR_BOTTOM_LEFT       = 9,
	ANCHOR_BOTTOM_RIGHT      = 10,
	ANCHOR_BOTTOM_LEFT_RIGHT = 11,
	ANCHOR_TOP_BOTTOM        = 12,
	ANCHOR_TOP_BOTTOM_LEFT   = 13,
	ANCHOR_TOP_BOTTOM_RIGHT  = 14,
	ANCHOR_ALL               = 15
};

struct SAnchorList
{
	int id;
	EAnchorStyle anchor;
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ダイアログウィンドウを扱うクラス

	ダイアログボックスを作るときにはここから継承させる．

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CDialog{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;

	using Me = CDialog;

public:
	
	struct RecentCombo final : public CCustomizedWnd {
		RecentCombo(int childId, CRecent& cRecent, bool font = false)
			: m_ChildId(childId)
			, m_cRecent(cRecent)
			, m_bCustomFont(font)
		{
		}

		bool	Attach(HWND hWnd, UINT uIdSubclass = 1) override;

		LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		int			m_ChildId;
		CRecent&	m_cRecent;
		bool		m_bCustomFont;
		FontHolder	m_hFont = nullptr;
	};

	/*
	||  Constructors
	*/
	CDialog( bool bSizable = false, bool bCheckShareData = true );
	CDialog(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CDialog(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CDialog();
	/*
	||  Attributes & Operations
	*/
	virtual INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */
	INT_PTR DoModal(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam);	/* モーダルダイアログの表示 */
	HWND DoModeless(HINSTANCE hInstance, HWND hwndParent, int nDlgTemplete, LPARAM lParam, int nCmdShow);	/* モードレスダイアログの表示 */
	HWND DoModeless(HINSTANCE hInstance, HWND hwndParent, LPCDLGTEMPLATE lpTemplate, LPARAM lParam, int nCmdShow);	/* モードレスダイアログの表示 */
	void CloseDialog(INT_PTR nModalRetVal);

	virtual BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	virtual void SetDialogPosSize();
	virtual BOOL OnDestroy( void );
	virtual BOOL OnNotify([[maybe_unused]] NMHDR* pNMHDR) { return FALSE; }
	BOOL OnSize();
	virtual BOOL OnSize( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnMove( WPARAM wParam, LPARAM lParam );
	virtual BOOL OnDrawItem( [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam ) { return TRUE; }
	virtual BOOL OnTimer( [[maybe_unused]] HWND hwnd, [[maybe_unused]] UINT uMsg, [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam ) { return TRUE; }
	virtual BOOL OnKeyDown( [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam ) { return TRUE; }
	virtual BOOL OnDeviceChange( [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam ) { return TRUE; }
	virtual int GetData( void ){return 1;}/* ダイアログデータの取得 */
	virtual void SetData( void ){return;}/* ダイアログデータの設定 */
	virtual BOOL OnBnClicked(int wID);
	virtual BOOL OnStnClicked( [[maybe_unused]] int wID ) { return FALSE; }
	virtual BOOL OnEnChange( [[maybe_unused]] HWND hwndCtl, [[maybe_unused]] int wID )     { return FALSE; }
	virtual BOOL OnEnSetFocus( [[maybe_unused]] HWND hwndCtl, [[maybe_unused]] int wID )   { return FALSE; }
	virtual BOOL OnEnKillFocus( [[maybe_unused]] HWND hwndCtl, [[maybe_unused]] int wID )  { return FALSE; }
	virtual BOOL OnLbnSelChange( [[maybe_unused]] HWND hwndCtl, [[maybe_unused]] int wID ) {  return FALSE; }
	virtual BOOL OnLbnDblclk( [[maybe_unused]] int wID ) { return FALSE; }
	virtual BOOL OnCbnSelChange( [[maybe_unused]] HWND hwndCtl, [[maybe_unused]] int wID ) { return FALSE; }
	virtual BOOL OnCbnEditChange( [[maybe_unused]] HWND hwndCtl, [[maybe_unused]] int wID ) { return FALSE; } // @@2005.03.31 MIK タグジャンプDialog
	virtual BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	static BOOL OnCbnDropDown( HWND hwndCtl, bool scrollBar );
	virtual BOOL OnCbnSelEndOk( HWND hwndCtl, int wID );

	virtual BOOL OnKillFocus( [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam )  { return FALSE; }
	virtual BOOL OnActivate( [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam )   { return FALSE; }	//@@@ 2003.04.08 MIK
	virtual int OnVKeyToItem( [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam )     { return -1; }
	virtual LRESULT OnCharToItem( [[maybe_unused]] WPARAM wParam, [[maybe_unused]] LPARAM lParam ) { return -1; }
	virtual BOOL OnPopupHelp(WPARAM wPara, LPARAM lParam);	//@@@ 2002.01.18 add
	virtual BOOL OnContextMenu(WPARAM wPara, LPARAM lParam);	//@@@ 2002.01.18 add
	virtual LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void ResizeItem( HWND hTarget, const POINT& ptDlgDefalut, const POINT& ptDlgNew, const RECT& rcItemDefault, EAnchorStyle anchor, bool bUpdate = true);
	void GetItemClientRect( int wID, RECT& rc );

public:

	static bool DirectoryUp(WCHAR* szDir);

public:
	HWND GetHwnd() const{ return m_hWnd; }
	//特殊インターフェース (使用は好ましくない)
	void _SetHwnd(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		m_hInstance = nullptr;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent = nullptr;	/* オーナーウィンドウのハンドル */
private:
	HWND			m_hWnd = nullptr;			/* このダイアログのハンドル */
	HFONT			m_hFontDialog;	// ダイアログに設定されているフォント(破棄禁止)
public:
	HWND			m_hwndSizeBox = nullptr;
	LPARAM			m_lParam = (LPARAM)nullptr;
	BOOL			m_bModal;		/* モーダル ダイアログか */
	bool			m_bSizable;		// 可変ダイアログかどうか
	int				m_nShowCmd = SW_SHOW;		//	最大化/最小化
	DLLSHAREDATA*	m_pShareData;
	BOOL			m_bInited;
	HINSTANCE		m_hLangRsrcInstance;		// メッセージリソースDLLのインスタンスハンドル	// 2011.04.10 nasukoji

protected:
	int				m_nWidth = -1;
	int				m_nHeight = -1;
	int				m_xPos = -1;
	int				m_yPos = -1;
	void CreateSizeBox( void );
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	HWND GetItemHwnd(int nID){ return ::GetDlgItem( GetHwnd(), nID ); }

	// コントロールに画面のフォントを設定	2012/11/27 Uchi
	HFONT SetMainFont( HWND hTarget );
	// このダイアログに設定されているフォントを取得
	HFONT GetDialogFont() { return m_hFontDialog; }
};

#endif /* SAKURA_CDIALOG_17C8C15C_881C_4C1F_B953_CB11FCC8B70B_H_ */
