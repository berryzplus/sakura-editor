/*!	@file
	@brief 分割線ウィンドウクラス

	@author Norio Nakatani
	@date 1998/07/07 新規作成
	@date 2002/2/3 aroka 未使用コード除去
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CSplitterWnd.h"

#include "window/CSplitBoxWnd.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"
#include "outline/CDlgFuncList.h"
#include "env/DLLSHAREDATA.h"
#include "uiparts/CGraphics.h"
#include "apiwrap/StdApi.h"
#include "CSelectLang.h"
#include "config/system_constants.h"
#include "String_define.h"

constexpr auto SPLITTER_FRAME_WIDTH = 3;
constexpr auto SPLITTER_MARGIN = 2;

/*!
 * 分割位置をチェックする
 *
 * @param[in] value 分割位置
 * @param[in] max 最大値
 * @param[in] margin マージン
 * @retval != 0 分割位置
 * @retval == 0 分割を解除する
 */
int CheckMargin(int value, int max, int margin) {
	if (value < margin || max - margin * 2 < value) {
		value = 0;
	}
	return value;
}

CSplitterWnd::CSplitterWnd()
: CWnd(L"::CSplitterWnd")
{
	auto& cLayoutMgr = GetDocument()->m_cLayoutMgr;
	cLayoutMgr.SetLayoutInfo( true, false, GetDocument()->m_cDocType.GetDocumentAttribute(),
		cLayoutMgr.GetTabSpaceKetas(), cLayoutMgr.m_tsvInfo.m_nTsvMode,
		cLayoutMgr.GetMaxLineKetas(), CLayoutXInt( -1 ), &GetEditWnd().GetLogfont() );
}

CEditDoc* CSplitterWnd::GetDocument() const
{
	return CEditDoc::getInstance();
}

/* 初期化 */
HWND CSplitterWnd::Create( HWND hwndParent )
{
	LPCWSTR pszClassName = L"SplitterWndClass";

	/* 初期化 */
	/* ウィンドウクラス作成 */
	ATOM atWork;
	atWork = RegisterWC(
		G_AppInstance(),
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		NULL,// Handle to the class cursor.
		(HBRUSH)NULL,// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated 
				//character string that specifies the resource name of the class menu,
				//as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);
	if( 0 == atWork ){
		ErrorMessage( NULL, LS(STR_ERR_CSPLITTER01) );
	}

	/* 基底クラスメンバ呼び出し */
	return CWnd::Create(
		hwndParent,
		0, // extended window style
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);
}

std::vector<std::tuple<int, RECT>> CSplitterWnd::CalcChildren(int cx, int cy) const
{
	const auto nFrameHeight = DpiScaleY(SPLITTER_FRAME_WIDTH);
	const auto nFrameWidth = DpiScaleX(SPLITTER_FRAME_WIDTH);

	std::vector<std::tuple<int, RECT>> rcChildren;

	if (m_nAllSplitRows == 1 && m_nAllSplitCols == 1) {
		rcChildren.emplace_back(0, RECT{ 0, 0, cx, cy });
	} else if (m_nAllSplitRows == 2 && m_nAllSplitCols == 1) {
		rcChildren.emplace_back(0, RECT{ 0, 0, cx, m_nVSplitPos });
		rcChildren.emplace_back(2, RECT{ 0, m_nVSplitPos + nFrameHeight, cx, cy - (m_nVSplitPos + nFrameHeight) });
	} else if (m_nAllSplitRows == 1 && m_nAllSplitCols == 2) {
		rcChildren.emplace_back(0, RECT{ 0, 0, m_nHSplitPos, cy });
		rcChildren.emplace_back(1, RECT{ m_nHSplitPos + nFrameWidth, 0, cx - (m_nHSplitPos + nFrameWidth), cy });
	} else {
		rcChildren.emplace_back(0, RECT{ 0, 0, m_nHSplitPos, m_nVSplitPos });
		rcChildren.emplace_back(1, RECT{ m_nHSplitPos + nFrameWidth, 0, cx - (m_nHSplitPos + nFrameWidth), cy });
		rcChildren.emplace_back(2, RECT{ 0, m_nVSplitPos + nFrameHeight, m_nHSplitPos, cy - (m_nVSplitPos + nFrameHeight) });
		rcChildren.emplace_back(3, RECT{ m_nHSplitPos + nFrameWidth, m_nVSplitPos + nFrameHeight, cx - (m_nHSplitPos + nFrameWidth), cy - (m_nVSplitPos + nFrameHeight) });
	}

	return rcChildren;
}

void CSplitterWnd::CreatePane(int index, const std::vector<std::tuple<int, RECT>>& rcChildren)
{
	const auto hWnd = GetHwnd();
	const auto cend = rcChildren.cend();
	const auto found = std::find_if(rcChildren.cbegin(), cend, [index]( const auto& rc ) { return std::get<int>(rc) == index; });
	if (found != cend && !m_ChildWndArr[index]) {
		m_ChildWndArr[index] = std::make_unique<CEditView>(index);
		m_ChildWndArr[index]->Create(hWnd, GetDocument(), index, FALSE, false );
		m_nChildWndCount++;
	}
}

/* 分割フレーム描画 */
void CSplitterWnd::DrawFrame( HDC hdc, RECT* prc )
{
	CSplitBoxWnd::Draw3dRect( hdc, prc->left, prc->top, prc->right, prc->bottom,
		::GetSysColor( COLOR_3DSHADOW ),
		::GetSysColor( COLOR_3DHILIGHT )
	);
	CSplitBoxWnd::Draw3dRect( hdc, prc->left + 1, prc->top + 1, prc->right - 2, prc->bottom - 2,
		RGB( 0, 0, 0 ),
		::GetSysColor( COLOR_3DFACE )
	);
	return;
}

/* 分割トラッカーの表示 */
void CSplitterWnd::DrawSplitter( int xPos, int yPos, int bEraseOld )
{
	HDC			hdc;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	RECT		rc;
	RECT		rc2;
	const int	nTrackerWidth = DpiScaleX(6);

	hdc = ::GetDC( GetHwnd() );
	hBrush = ::CreateSolidBrush( RGB(255,255,255) );
	hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	::SetROP2( hdc, R2_XORPEN );
	::SetBkMode( hdc, TRANSPARENT );
	::GetClientRect( GetHwnd(), &rc );

	if( bEraseOld ){
		if( m_bDragging & 1 ){	/* 分割バーをドラッグ中か */
			rc2.left = -1;
			rc2.top = m_nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + nTrackerWidth;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
		}
		if( m_bDragging & 2 ){	/* 分割バーをドラッグ中か */
			rc2.left = m_nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + nTrackerWidth;
			rc2.bottom = rc.bottom;
			::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
		}
	}

	m_nDragPosX = xPos;
	m_nDragPosY = yPos;
	if( m_bDragging & 1 ){	/* 分割バーをドラッグ中か */
		rc2.left = -1;
		rc2.top = m_nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + nTrackerWidth;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
	}
	if( m_bDragging & 2 ){	/* 分割バーをドラッグ中か */
		rc2.left = m_nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + nTrackerWidth;
		rc2.bottom = rc.bottom;
		::Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
	}

	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
	::ReleaseDC( GetHwnd(), hdc );
	return;
}

/* 分割バーへのヒットテスト */
int CSplitterWnd::HitTestSplitter( int xPos, int yPos )
{
	const int	nFrameWidth = DpiScaleX(SPLITTER_FRAME_WIDTH);
	const int	nMargin = DpiScaleX(SPLITTER_MARGIN);

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		return 0;
	}else
	if( m_nAllSplitRows == 2 && m_nAllSplitCols == 1 ){
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin ){
			return 1;
		}else{
			return 0;
		}
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 2 ){
		if( m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 2;
		}else{
			return 0;
		}
	}else{
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin &&
			m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 3;
		}else
		if( m_nVSplitPos - nMargin < yPos && yPos < m_nVSplitPos + nFrameWidth + nMargin ){
			return 1;
		}else
		if( m_nHSplitPos - nMargin < xPos && xPos < m_nHSplitPos + nFrameWidth + nMargin ){
			return 2;
		}else{
			return 0;
		}
	}
}

/*! ウィンドウの分割
	@param nHorizontal 水平クライアント座標 1以上で分割 0:分割しない  -1: 前の設定を保持
	@param nVertical   垂直クライアント座標 1以上で分割 0:分割しない  -1: 前の設定を保持
*/
void CSplitterWnd::DoSplit( int nHorizontal, int nVertical )
{
	const auto hWnd = GetHwnd();

	if( -1 == nHorizontal && -1 == nVertical ){
		nVertical   = m_nVSplitPos;		/* 垂直分割位置 */
		nHorizontal = m_nHSplitPos;		/* 水平分割位置 */
	}

	nVertical = CheckMargin(nVertical, m_cy, DpiScaleY(32));
	nHorizontal = CheckMargin(nHorizontal, m_cx, DpiScaleX(32));

	m_nVSplitPos = nVertical;		/* 垂直分割位置 */
	m_nHSplitPos = nHorizontal;		/* 水平分割位置 */

	const auto nAllSplitRowsOld = m_nAllSplitRows;	/* 分割行数 */
	const auto nAllSplitColsOld = m_nAllSplitCols;	/* 分割桁数 */

	for (auto it = m_ChildWndArr.begin() + m_nChildWndCount; it != m_ChildWndArr.end(); ++it, ++m_nChildWndCount) {
		const auto index = int(it - m_ChildWndArr.begin());
		*it = std::make_unique<CEditView>(index);
	}

	const auto rcChildren = CalcChildren(m_cx, m_cy);
	for (const auto& [index, rcChild] : rcChildren) {
		CreatePane(index, rcChildren);
	}

	int nActivePane;

	// 分割なし
	if (!nVertical && !nHorizontal)
	{
		m_nAllSplitRows = 1;	/* 分割行数 */
		m_nAllSplitCols = 1;	/* 分割桁数 */

		m_ChildWndArr[0]->SplitBoxOnOff(TRUE, TRUE, FALSE); m_ChildWndArr[0]->Show(SW_SHOW);
		m_ChildWndArr[1]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[1]->Show(SW_HIDE);
		m_ChildWndArr[2]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[2]->Show(SW_HIDE);
		m_ChildWndArr[3]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[3]->Show(SW_HIDE);

		nActivePane = 0;
	}
	// 縦方向分割
	else if (nVertical && !nHorizontal)
	{
		m_nAllSplitRows = 2;	/* 分割行数 */
		m_nAllSplitCols = 1;	/* 分割桁数 */

		m_ChildWndArr[0]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[0]->Show(SW_SHOW);
		m_ChildWndArr[1]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[1]->Show(SW_HIDE);
		m_ChildWndArr[2]->SplitBoxOnOff(FALSE, TRUE, FALSE); m_ChildWndArr[2]->Show(SW_SHOW);
		m_ChildWndArr[3]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[3]->Show(SW_HIDE);

		if( nAllSplitRowsOld == 1 && nAllSplitColsOld == 1 ){
			/* ペインの表示状態を他のビューにコピー */
			m_ChildWndArr[0]->CopyViewStatus( m_ChildWndArr[2].get() );

			const auto viewTopLine = m_ChildWndArr[0]->GetTextArea().GetViewTopLine() + m_ChildWndArr[0]->GetTextArea().m_nViewRowNum;
			m_ChildWndArr[2]->GetTextArea().SetViewTopLine(viewTopLine);
		}

		if (m_nActivePane == 0 || m_nActivePane == 1) {
			nActivePane = 0;
		} else {
			nActivePane = 2;
		}
	}
	// 横方向分割
	else if (!nVertical && nHorizontal)
	{
		m_nAllSplitRows = 1;	/* 分割行数 */
		m_nAllSplitCols = 2;	/* 分割桁数 */

		m_ChildWndArr[0]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[0]->Show(SW_SHOW);
		m_ChildWndArr[1]->SplitBoxOnOff(TRUE, FALSE, FALSE); m_ChildWndArr[1]->Show(SW_SHOW);
		m_ChildWndArr[2]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[2]->Show(SW_HIDE);
		m_ChildWndArr[3]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[3]->Show(SW_HIDE);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
			/* ペインの表示状態を他のビューにコピー */
			m_ChildWndArr[0]->CopyViewStatus( m_ChildWndArr[1].get() );
		}

		if (m_nActivePane == 0 || m_nActivePane == 2) {
			nActivePane = 0;
		} else {
			nActivePane = 1;
		}
	}
	// 縦横分割
	else
	{
		m_nAllSplitRows = 2;	/* 分割行数 */
		m_nAllSplitCols = 2;	/* 分割桁数 */

		m_ChildWndArr[0]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[0]->Show(SW_SHOW);
		m_ChildWndArr[1]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[1]->Show(SW_SHOW);
		m_ChildWndArr[2]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[2]->Show(SW_SHOW);
		m_ChildWndArr[3]->SplitBoxOnOff(FALSE, FALSE, FALSE); m_ChildWndArr[3]->Show(SW_SHOW);

		nActivePane = _DoSplitMax(nAllSplitColsOld, nAllSplitRowsOld);
	}

	DispatchEvent(hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(m_cx, m_cy));

	/* アクティブになったことをペインに通知 */
	if( m_ChildWndArr[nActivePane] != NULL ){
		PostMessageW( m_ChildWndArr[nActivePane]->GetHwnd(), MYWM_SETACTIVEPANE, 0, 0 );
	}
}

int CSplitterWnd::_DoSplitMax(int nAllSplitColsOld, int nAllSplitRowsOld)
{
	if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
		/* ペインの表示状態を他のビューにコピー */
		m_ChildWndArr[0]->CopyViewStatus( m_ChildWndArr[1].get() );
		m_ChildWndArr[0]->CopyViewStatus( m_ChildWndArr[2].get() );
		m_ChildWndArr[0]->CopyViewStatus( m_ChildWndArr[3].get() );
	} else if (nAllSplitColsOld == 1) {
		/* ペインの表示状態を他のビューにコピー */
		m_ChildWndArr[0]->CopyViewStatus( m_ChildWndArr[1].get() );
		m_ChildWndArr[2]->CopyViewStatus( m_ChildWndArr[3].get() );
	} else if (nAllSplitRowsOld == 1) {
		/* ペインの表示状態を他のビューにコピー */
		m_ChildWndArr[0]->CopyViewStatus( m_ChildWndArr[2].get() );
		m_ChildWndArr[1]->CopyViewStatus( m_ChildWndArr[3].get() );
	}

	return m_nActivePane;
}

/* アクティブペインの設定 */
void CSplitterWnd::SetActivePane( int nIndex )
{
	assert( nIndex < MAXCOUNTOFVIEW );
	m_nActivePane = nIndex;
	return;
}

/* 縦分割ＯＮ／ＯＦＦ */
void CSplitterWnd::VSplitOnOff( void )
{
	RECT		rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		DoSplit( 0, rc.bottom / 2 );
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols > 1 ){
		DoSplit( m_nHSplitPos, rc.bottom / 2 );
	}else
	if( m_nAllSplitRows > 1 && m_nAllSplitCols == 1 ){
		DoSplit( 0, 0 );
	}else{
		DoSplit( m_nHSplitPos, 0 );
	}
	return;
}

/* 横分割ＯＮ／ＯＦＦ */
void CSplitterWnd::HSplitOnOff( void )
{
	RECT		rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows == 1 && m_nAllSplitCols == 1 ){
		DoSplit( rc.right / 2, 0 );
	}else
	if( m_nAllSplitRows == 1 && m_nAllSplitCols > 1 ){
		DoSplit( 0, 0 );
	}else
	if( m_nAllSplitRows > 1 && m_nAllSplitCols == 1 ){
		DoSplit( rc.right / 2 , m_nVSplitPos );
	}else{
		DoSplit( 0, m_nVSplitPos );
	}
	return;
}

/* 縦横分割ＯＮ／ＯＦＦ */
void CSplitterWnd::VHSplitOnOff( void )
{
	int		nX;
	int		nY;
	RECT	rc;
	::GetClientRect( GetHwnd(), &rc );

	if( m_nAllSplitRows > 1 && m_nAllSplitCols > 1 ){
		nX = 0;
		nY = 0;
	}else{
		if( m_nAllSplitRows == 1){
			nY = rc.bottom / 2;
		}else{
			nY = m_nVSplitPos;
		}
		if( m_nAllSplitCols == 1 ){
			nX = rc.right / 2;
		}else{
			nX = m_nHSplitPos;
		}
	}
	DoSplit( nX, nY );

	return;
}

/* 前のペインを返す */
int CSplitterWnd::GetPrevPane( void )
{
	int		nPane;
	nPane = -1;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = -1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 2:
			nPane = 0;
			break;
		}
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		}
	}else{
		switch( m_nActivePane ){
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		case 2:
			nPane = 1;
			break;
		case 3:
			nPane = 2;
			break;
		}
	}
	return nPane;
}

/* 次のペインを返す */
int CSplitterWnd::GetNextPane( void )
{
	int		nPane;
	nPane = -1;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = -1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		switch( m_nActivePane ){
		case 0:
			nPane = 2;
			break;
		case 2:
			nPane = -1;
			break;
		}
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		switch( m_nActivePane ){
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = -1;
			break;
		}
	}else{
		switch( m_nActivePane ){
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = 2;
			break;
		case 2:
			nPane = 3;
			break;
		case 3:
			nPane = -1;
			break;
		}
	}
	return nPane;
}

/* 最初のペインを返す */
int CSplitterWnd::GetFirstPane( void ) const
{
	return 0;
}

/* 最後のペインを返す */
int CSplitterWnd::GetLastPane( void ) const
{
	int		nPane;
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 1 ){
		nPane = 0;
	}else
	if( m_nAllSplitRows == 1 &&	m_nAllSplitCols == 2 ){
		nPane = 1;
	}else
	if( m_nAllSplitRows == 2 &&	m_nAllSplitCols == 1 ){
		nPane = 2;
	}else{
		nPane = 3;
	}
	return nPane;
}

/* 描画処理 */
LRESULT CSplitterWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	RECT		rcFrame;
	const int	nFrameWidth = DpiScaleX(SPLITTER_FRAME_WIDTH);
	hdc = ::BeginPaint( hwnd, &ps );
	::GetClientRect( GetHwnd(), &rc );
	if( m_nAllSplitRows > 1 ){
		::SetRect( &rcFrame, rc.left, m_nVSplitPos, rc.right, m_nVSplitPos + nFrameWidth );
		::MyFillRect( hdc, rcFrame, COLOR_3DFACE );
	}
	if( m_nAllSplitCols > 1 ){
		::SetRect( &rcFrame, m_nHSplitPos, rc.top, m_nHSplitPos + nFrameWidth, rc.bottom );
		::MyFillRect( hdc, rcFrame, COLOR_3DFACE );
	}
	::EndPaint(hwnd, &ps);
	return 0L;
}

/* マウス移動時の処理 */
LRESULT CSplitterWnd::OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int		nHit;
	RECT	rc;
	int		xPos;
	int		yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	nHit = HitTestSplitter( xPos, yPos );
	switch( nHit ){
	case 1:
		::SetCursor( ::LoadCursor( NULL, IDC_SIZENS ) );
		break;
	case 2:
		::SetCursor( ::LoadCursor( NULL, IDC_SIZEWE ) );
		break;
	case 3:
		::SetCursor( ::LoadCursor( NULL, IDC_SIZEALL ) );
		break;
	}
	if( 0 != m_bDragging ){		/* 分割バーをドラッグ中か */
		::GetClientRect( GetHwnd(), &rc );
		const int n1 = DpiScaleX(1);
		const int n6 = DpiScaleX(6);
		if( xPos < n1 ){
			xPos = n1;
		}
		if( xPos > rc.right - n6 ){
			xPos = rc.right - n6;
		}
		if( yPos < n1 ){
			yPos = n1;
		}
		if( yPos > rc.bottom - n6 ){
			yPos = rc.bottom - n6;
		}
		/* 分割トラッカーの表示 */
		DrawSplitter( xPos, yPos, TRUE );
//		MYTRACE( L"xPos=%d yPos=%d \n", xPos, yPos );
	}
	return 0L;
}

/* マウス左ボタン押下時の処理 */
LRESULT CSplitterWnd::OnLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int		nHit;
	int		xPos;
	int		yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	::SetFocus( GetParentHwnd() );
	/* 分割バーへのヒットテスト */
	nHit = HitTestSplitter( xPos, yPos );
	if( 0 != nHit ){
		m_bDragging = nHit;	/* 分割バーをドラッグ中か */
		::SetCapture( GetHwnd() );
	}
	/* 分割トラッカーの表示 */
	DrawSplitter( xPos, yPos, FALSE );

	return 0L;
}

/* マウス左ボタン解放時の処理 */
LRESULT CSplitterWnd::OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int bDraggingOld;
	int nX;
	int nY;

	if( m_bDragging ){
		/* 分割トラッカーの表示 */
		DrawSplitter( m_nDragPosX, m_nDragPosY, FALSE );
		bDraggingOld = m_bDragging;
		m_bDragging = 0;
		::ReleaseCapture();
		if( NULL != m_hcurOld ){
			::SetCursor( m_hcurOld );
		}
		/* ウィンドウの分割 */
		if( m_nAllSplitRows == 1 ){
			nY = 0;
		}else{
			nY = m_nDragPosY;
		}
		if( m_nAllSplitCols == 1 ){
			nX = 0;
		}else{
			nX = m_nDragPosX;
		}
		if( bDraggingOld == 1 ){
			DoSplit( m_nHSplitPos, nY );
		}else
		if( bDraggingOld == 2 ){
			DoSplit( nX, m_nVSplitPos );
		}else
		if( bDraggingOld == 3 ){
			DoSplit( nX, nY );
		}
	}
	return 0L;
}

/* マウス左ボタンダブルクリック時の処理 */
LRESULT CSplitterWnd::OnLButtonDblClk( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int nX;
	int nY;
	int	nHit;
	int	xPos;
	int	yPos;

	xPos = (int)(short)LOWORD(lParam);
	yPos = (int)(short)HIWORD(lParam);

	nHit = HitTestSplitter( xPos, yPos );
	if( nHit == 1 ){
		if( m_nAllSplitCols == 1 ){
			nX = 0;
		}else{
			nX = m_nHSplitPos;
		}
		DoSplit( nX , 0 );
	}else
	if( nHit == 2 ){
		if( m_nAllSplitRows == 1 ){
			nY = 0;
		}else{
			nY = m_nVSplitPos;
		}
		DoSplit( 0 , nY );
	}else
	if( nHit == 3 ){
		DoSplit( 0 , 0 );
	}
	OnMouseMove( GetHwnd(), 0, 0, MAKELONG( xPos, yPos ) );
	return 0L;
}

/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
LRESULT CSplitterWnd::DispatchEvent_WM_APP( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int nPosX;
	int nPosY;
	switch( uMsg ){
	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE( L"MYWM_DOSPLIT nPosX=%d nPosY=%d\n", nPosX, nPosY );

		/* ウィンドウの分割 */
		if( 0 != m_nHSplitPos ){
			nPosX = m_nHSplitPos;
		}
		if( 0 != m_nVSplitPos ){
			nPosY = m_nVSplitPos;
		}
		DoSplit( nPosX , nPosY );
		break;
	case MYWM_SETACTIVEPANE:
		SetActivePane( (int)wParam );
		break;
	}
	return 0L;
}

/*!
 * WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 * メッセージの戻り値はウインドウの作成を続行するかどうかの判断に使われます。
 *
 * @retval true  ウィンドウの作成を続行する
 * @retval false ウィンドウの作成を中止する
 */
bool CSplitterWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    if (!__super::OnCreate(hWnd, lpCreateStruct))
    {
        return false;
    }

	// 最初のビューを作成
	GetActiveView().Create(hWnd, GetDocument(), 0, TRUE, false);
	GetActiveView().SplitBoxOnOff(TRUE, TRUE, FALSE);
	GetActiveView().OnSetFocus();

	return true;
}

/*!
 * WM_SIZEハンドラ
 *
 * ウィンドウサイズの変更処理
 */
void CSplitterWnd::OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	__super::OnSize(hWnd, state, cx, cy);

	const auto rcChildren = CalcChildren(cx, cy);

	if (auto hdwp = BeginDeferWindowPos(int(rcChildren.size())))
	{
		for (const auto& [index, rcChild] : rcChildren) {
			hdwp = DeferWindowPos(
				hdwp,
				m_ChildWndArr[index]->GetHwnd(),
				nullptr,
				rcChild.left,
				rcChild.top,
				rcChild.right - rcChild.left,
				rcChild.bottom - rcChild.top,
				SWP_NOZORDER | SWP_NOACTIVATE
			);
			if (!hdwp) {
				break;
			}
		}

		if (hdwp) {
			EndDeferWindowPos(hdwp);
		}
	}
}
