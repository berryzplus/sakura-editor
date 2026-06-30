/*!	@file
	@brief タブウィンドウ

	@author MIK
	@date 2003.5.30
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2004, MIK, Kazika
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2012, Moca, syat
	Copyright (C) 2013, Uchi, aroka, novice, syat, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CTABWND_E95D57BD_51E6_467A_9F6D_2C68BF122449_H_
#define SAKURA_CTABWND_E95D57BD_51E6_467A_9F6D_2C68BF122449_H_
#pragma once

#include "env/CommonSetting.h"
#include "util/design_template.h"
#include "window/CWnd.h"

class CGraphics;
struct EditNode;
struct DLLSHAREDATA;

//! タブバーウィンドウ
class CTabWnd final : public TSizeBoxParent<COriginalWnd>
{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;
	using ImageListHolder = cxx::ResourceHolder<&::ImageList_Destroy>;

	using Base = TSizeBoxParent<COriginalWnd>;
	using Me = CTabWnd;

public:
	enum DragState { DRAG_NONE, DRAG_CHECK, DRAG_DRAG };
	enum CaptureSrc { CAPT_NONE, CAPT_CLOSE };

	struct TabCtrl final : public TCustomizedCtrl<CTabWnd> {
		explicit TabCtrl(CTabWnd& tabWnd) : TCustomizedCtrl(tabWnd) {}

		HWND	GetParentHwnd() const noexcept { return m_ParentWnd.GetHwnd(); }

		LRESULT DefTabWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const {
			return DefWndProcW(hWnd, uMsg, wParam, lParam);
		}

		void	BreakDrag() { m_ParentWnd.BreakDrag(); }
		LRESULT	ExecTabCommand( int nId, POINTS pts );	/*!< タブ部 コマンド実行処理 */

		int		SetCarmWindowPlacement( HWND hwnd, const WINDOWPLACEMENT* pWndpl );	/* アクティブ化の少ない SetWindowPlacement() を実行する */	// 2007.11.30 ryoji
		void	ForceActiveWindow( HWND hwnd );

		void	BroadcastRefreshToGroup() { m_ParentWnd.BroadcastRefreshToGroup(); }
		void	Refresh( BOOL bEnsureVisible = TRUE, BOOL bRebuild = FALSE ) { m_ParentWnd.Refresh(bEnsureVisible, bRebuild); }
		BOOL	ReorderTab( int nSrcTab, int nDstTab ) { return m_ParentWnd.ReorderTab(nSrcTab, nDstTab); }
		BOOL	SeparateGroup( HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop ) { return m_ParentWnd.SeparateGroup(hwndSrc, hwndDst, ptDrag, ptDrop); }

		LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		void	OnTimer(HWND hWnd, UINT id) override;
		LRESULT	OnMouseMove(WPARAM wParam, LPARAM lParam);
		LRESULT	OnLButtonDown(WPARAM wParam, LPARAM lParam);
		LRESULT	OnLButtonUp(WPARAM wParam, LPARAM lParam);
		void	OnRButtonUp(HWND hWnd, int x, int y, UINT keyFlags);
		void	OnMButtonUp(HWND hWnd, int x, int y, UINT keyFlags);
		void	OnCaptureChanged(HWND hWnd, HWND hWndCapture);

		DLLSHAREDATA*	m_pShareData = &::GetDllShareData();

		HWND&			m_hwndTab = m_hWnd;

		BOOL			m_bVisualStyle = FALSE;			//!< ビジュアルスタイルかどうか	// 2007.04.01 ryoji

		DragState		m_eDragState = DRAG_NONE;			//!< ドラッグ状態
		int				m_nSrcTab = 0;				//!< 移動元タブ
		POINT			m_ptSrcCursor{};			//!< ドラッグ開始カーソル位置
		HCURSOR			m_hDefaultCursor = nullptr;		//!< ドラッグ開始時のカーソル

		BOOL&			m_bListBtnHilighted = m_ParentWnd.m_bListBtnHilighted;
		BOOL&			m_bCloseBtnHilighted = m_ParentWnd.m_bCloseBtnHilighted;	//!< 閉じるボタンハイライト状態	// 2006.10.21 ryoji
		CaptureSrc&		m_eCaptureSrc = m_ParentWnd.m_eCaptureSrc;			//!< キャプチャ元
		BOOL			m_bTabSwapped = FALSE;			//!< ドラッグ中にタブの入れ替えがあったかどうか
		LONG*			m_nTabBorderArray = nullptr;		//!< ドラッグ前のタブ境界位置配列
		LOGFONT			m_lf{};					//!< 表示フォントの特性情報

		// タブ内の閉じるボタン用変数
		int&			m_nTabHover = m_ParentWnd.m_nTabHover;			//!< マウスカーソル下のタブ（無いときは-1）
		bool			m_bTabCloseHover = false;		//!< マウスカーソル下にタブ内の閉じるボタンがあるか
		int				m_nTabCloseCapture = -1;		//!< 閉じるボタンがマウス押下されているタブ（無いときは-1）
	};

	/*
	||  Constructors
	*/
	CTabWnd();
	~CTabWnd() override;

	/*
	|| メンバ関数
	*/
	HWND	Open(HWND hWndParent, CMyRect& rc);

	void TabWindowNotify( WPARAM wParam, LPARAM lParam );
	void Refresh( BOOL bEnsureVisible = TRUE, BOOL bRebuild = FALSE );			// 2006.02.06 ryoji 引数削除
	void NextGroup( void );			/* 次のグループ */			// 2007.06.20 ryoji
	void PrevGroup( void );			/* 前のグループ */			// 2007.06.20 ryoji
	void MoveRight( void );			/* タブを右に移動 */		// 2007.06.20 ryoji
	void MoveLeft( void );			/* タブを左に移動 */		// 2007.06.20 ryoji
	void Separate( void );			/* 新規グループ */			// 2007.06.20 ryoji
	void JoinNext( void );			/* 次のグループに移動 */	// 2007.06.20 ryoji
	void JoinPrev( void );			/* 前のグループに移動 */	// 2007.06.20 ryoji

	LRESULT TabListMenu( POINT pt, BOOL bSel = TRUE, BOOL bFull = FALSE, BOOL bOtherGroup = TRUE );	/*!< タブ一覧メニュー作成処理 */	// 2006.03.23 fon

	void OnSize(){
		OnSize( GetHwnd(), WM_SIZE, 0, 0 );
	}
	void UpdateStyle();
	void UpdateTheme();		/*!< ダークモード切替時のテーマ更新 */

	/*
	|| 実装ヘルパ系
	*/
	int FindTabIndexByHWND( HWND hWnd );
	void AdjustWindowPlacement( void );							/*!< 編集ウィンドウの位置合わせ */	// 2007.04.03 ryoji
	void HideOtherWindows( HWND hwndExclude );					/*!< 他の編集ウィンドウを隠す */	// 2007.05.17 ryoji
	void ForceActiveWindow( HWND hwnd );
	HWND GetNextGroupWnd( void );	/* 次のグループの先頭ウィンドウを探す */	// 2007.06.20 ryoji
	HWND GetPrevGroupWnd( void );	/* 前のグループの先頭ウィンドウを探す */	// 2007.06.20 ryoji
	void GetTabName( EditNode* pEditNode, BOOL bFull, BOOL bDupamp, LPWSTR pszName, int nLen );	/* タブ名取得処理 */	// 2007.06.28 ryoji 新規作成

	/* 仮想関数 */
	LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	/* 仮想関数 メッセージ処理 */
	bool	OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) override;
	void	OnDestroy(HWND hWnd) override;
	void	OnSize(HWND hWnd, UINT state, int cx, int cy) override;
	void	OnPaint(HWND hWnd, PAINTSTRUCT& ps) override;
	void	OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT* lpDrawItem) override;
	void	OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* lpMeasureItem) override;

	LRESULT	OnNotify(HWND hWnd, UINT_PTR idFrom, LPNMHDR pNMHDR) override;

	void	OnTimer(HWND hWnd, UINT id) override;
	void	OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags) override;
	void	OnLButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags) override;
	void	OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags) override;
	void	OnLButtonDblClk(HWND hWnd, int x, int y, UINT keyFlags) override;
	void	OnRButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags) override;

	//実装補助インターフェース
	void BreakDrag(HWND hWnd = nullptr, bool fDoubleClick = false, int x = 0, int y = 0, UINT keyFlags = 0);
	BOOL ReorderTab( int nSrcTab, int nDstTab );	/*!< タブ順序変更処理 */
	void BroadcastRefreshToGroup( void );
	BOOL SeparateGroup( HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop );	/*!< タブ分離処理 */	// 2007.06.20 ryoji
	bool	LayoutTab(int cx);

	HIMAGELIST InitImageList( void );				/*!< イメージリストの初期化処理 */
	int GetImageIndex( EditNode* pNode );			/*!< イメージリストのインデックス取得処理 */

	int		SetCarmWindowPlacement( HWND hwnd, const WINDOWPLACEMENT* pWndpl ) { return m_TabCtrl.SetCarmWindowPlacement(hwnd, pWndpl); }

	/*
	|| メンバ変数
	*/
	DLLSHAREDATA*	m_pShareData = &GetDllShareData();	/*!< 共有データ */
	FontHolder		m_hFont = nullptr;			//!< 表示用フォント
	HWND			m_hwndTab = nullptr;		/*!< タブコントロール */
	HWND			m_hwndToolTip = nullptr;	/*!< ツールチップ（ボタン用） */
	WCHAR			m_szTextTip[1024];	/*!< ツールチップのテキスト（タブ用） */
	ETabPosition	m_eTabPosition = TabPosition_None;	//!< タブ表示位置

	ImageListHolder	m_hIml = nullptr;					//!< イメージリスト
	HICON		m_hIconApp;				//!< アプリケーションアイコン
	HICON		m_hIconGrep;			//!< Grepアイコン
	int			m_iIconApp;				//!< アプリケーションアイコンのインデックス
	int			m_iIconGrep;			//!< Grepアイコンのインデックス

	BOOL		m_bHovering = FALSE;
	BOOL		m_bListBtnHilighted = FALSE;
	BOOL		m_bCloseBtnHilighted = FALSE;	//!< 閉じるボタンハイライト状態	// 2006.10.21 ryoji
	CaptureSrc	m_eCaptureSrc = CAPT_NONE;			//!< キャプチャ元
	bool		m_bMultiLine;			//!< 複数行

	// タブ内の閉じるボタン用変数
	int			m_nTabHover = -1;			//!< マウスカーソル下のタブ（無いときは-1）
	bool		m_bTabCloseHover = false;		//!< マウスカーソル下にタブ内の閉じるボタンがあるか

	TabCtrl		m_TabCtrl{ *this };

	DISALLOW_COPY_AND_ASSIGN(CTabWnd);
};

#endif /* SAKURA_CTABWND_E95D57BD_51E6_467A_9F6D_2C68BF122449_H_ */
