/*!	@file
	@brief 分割線ウィンドウクラス

	@author Norio Nakatani
	@date 1998/07/07 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CSPLITTERWND_8F27B39C_B96B_4964_ACD8_E157A146F892_H_
#define SAKURA_CSPLITTERWND_8F27B39C_B96B_4964_ACD8_E157A146F892_H_
#pragma once

#include "CWnd.h"
#include "env/SShareDataClientWithCache.hpp"

#include "view/CEditView.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
	@brief 分割線ウィンドウクラス
	
	４分割ウィンドウの管理と分割線の描画を行う。
*/
class CSplitterWnd final : private SShareDataClientWithCache, public CWnd
{
	static constexpr auto MAXCOUNTOFVIEW = 4;
	using CEditViewHolder = std::unique_ptr<CEditView>;
	using CEditViewsHolder = std::array<CEditViewHolder, MAXCOUNTOFVIEW>;

public:
	/*
	||  Constructors
	*/
	CSplitterWnd();

private: // 2002/2/3 aroka
	/*
	||  Attributes & Operations
	*/
	CEditViewsHolder    m_ChildWndArr = { std::make_unique<CEditView>(0) };
	int                 m_nChildWndCount = 1;		/*!< 有効な子ウィンドウ配列の数 */

	int				    m_nAllSplitRows = 1;		/* 分割行数 */
	int				    m_nAllSplitCols = 1;		/* 分割桁数 */
	int				    m_nVSplitPos    = 0;		/* 垂直分割位置 */
	int				    m_nHSplitPos    = 0;		/* 水平分割位置 */
	HCURSOR			    m_hcurOld       = nullptr;	/* もとのマウスカーソル */
	int				    m_bDragging     = false;	/* 分割バーをドラッグ中か */
	int				    m_nDragPosX     = 0;		/* ドラッグ位置Ｘ */
	int				    m_nDragPosY     = 0;		/* ドラッグ位置Ｙ */
	int				    m_nActivePane   = 0;		/* アクティブなペイン */

public: // 2002/2/3 aroka
	HWND Create( HWND hwndParent );	/* 初期化 */

	CEditView&			GetActiveView() const { return *m_ChildWndArr[m_nActivePane]; }
	CEditView&          GetView(int n) const { return *m_ChildWndArr[n]; }
	bool                IsPaneEnabled(int n) const { return 0 <= n && n < m_nChildWndCount; }
	int                 CountPanes() const { return m_nChildWndCount; }

	void DoSplit(int nHorizontal, int nVertical);	/* ウィンドウの分割 */
	void SetActivePane(int nIndex);	/* アクティブペインの設定 */
	int GetPrevPane( void );	/* 前のペインを返す */
	int GetNextPane( void );	/* 次のペインを返す */
	int GetFirstPane( void );	/* 最初のペインを返す */
	int GetLastPane( void );	/* 最後のペインを返す */

	void VSplitOnOff( void );	/* 縦分割ＯＮ／ＯＦＦ */
	void HSplitOnOff( void );	/* 横分割ＯＮ／ＯＦＦ */
	void VHSplitOnOff( void );	/* 縦横分割ＯＮ／ＯＦＦ */
//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */
	int GetAllSplitRows() const { return m_nAllSplitRows;} // 2002/2/3 aroka
	int GetAllSplitCols() const { return m_nAllSplitCols;} // 2002/2/3 aroka

	bool    OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) override;

protected:
	/* 仮想関数 */
	LRESULT DispatchEvent_WM_APP(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;	/* ウィンドウサイズの変更処理 */
	LRESULT OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;	/* 描画処理 */
	LRESULT OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override; /* マウス移動時の処理 */
	LRESULT OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;	/* マウス左ボタン押下時の処理 */
	LRESULT OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;	/* マウス左ボタン解放時の処理 */
	LRESULT OnLButtonDblClk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;	/* マウス左ボタンダブルクリック時の処理 */
	/*
	||  実装ヘルパ関数
	*/
	void DrawFrame(HDC hdc, RECT* prc);	/* 分割フレーム描画 */
	int HitTestSplitter(int xPos, int yPos);	/* 分割バーへのヒットテスト */
	void DrawSplitter(int xPos, int yPos, int bEraseOld);	/* 分割トラッカーの表示 */

	bool CreateEditViewBySplit(int nViewCount);

	CEditDoc* GetDocument() const;
};

#endif /* SAKURA_CSPLITTERWND_8F27B39C_B96B_4964_ACD8_E157A146F892_H_ */
