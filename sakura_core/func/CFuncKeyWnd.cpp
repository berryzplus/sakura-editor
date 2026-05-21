/*!	@file
	@brief ファンクションキーウィンドウ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, MIK, Moca
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2004, novice
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "func/CFuncKeyWnd.h"

#include "env/CShareData.h"
#include "window/CEditWnd.h"
#include "doc/CEditDoc.h"
#include "util/input.h"
#include "util/window.h"
#include "apiwrap/StdApi.h"
#include "apiwrap/StdControl.h"

#define IDT_FUNCWND 1248
#define TIMER_TIMEOUT 100
#define TIMER_CHECKFUNCENABLE 300

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CFuncKeyWnd::CFuncKeyWnd()
	: COriginalWnd(L"CFuncKeyWnd")
{
	int		i;
	LOGFONT	lf;
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();
	for( i = 0; i < int(std::size(m_szFuncNameArr)); ++i ){
		m_szFuncNameArr[i][0] = L'\0';
	}
//	2002.11.04 Moca Open()側で設定
//	m_nButtonGroupNum = 4;

	for( i = 0; i < int(std::size(m_hwndButtonArr)); ++i ){
		m_hwndButtonArr[i] = nullptr;
	}

	/* 表示用フォント */
	/* LOGFONTの初期化 */
	memset_raw( &lf, 0, sizeof(lf) );
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji 高DPI対応（ポイント数から算出）
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	wcscpy( lf.lfFaceName, L"ＭＳ Ｐゴシック" );
	m_hFont = ::CreateFontIndirect( &lf );

	return;
}

CFuncKeyWnd::~CFuncKeyWnd()
{
	return;
}

/* ウィンドウ オープン */
HWND CFuncKeyWnd::Open( HINSTANCE hInstance, HWND hwndParent, CEditDoc* pCEditDoc, bool bSizeBox )
{
	LPCWSTR pszClassName = m_ClassName.c_str();

	m_pcEditDoc = pCEditDoc;
	m_bSizeBox = bSizeBox;
	m_hwndSizeBox = nullptr;
	m_nCurrentKeyState = -1;

	// 2002.11.04 Moca 変更できるように
	m_nButtonGroupNum = m_pShareData->m_Common.m_sWindow.m_nFUNCKEYWND_GroupNum;
	if( 1 > m_nButtonGroupNum || 12 < m_nButtonGroupNum ){
		m_nButtonGroupNum = 4;
	}

	/* ウィンドウクラス作成 */
	RegisterWC(
		hInstance,
		nullptr,// Handle to the class icon.
		nullptr,	//Handle to a small icon
		::LoadCursor( nullptr, IDC_ARROW ),// Handle to the class cursor.
		(HBRUSH)(COLOR_3DFACE + 1),// Handle to the class background brush.
		nullptr/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);

	/* 基底クラスメンバ呼び出し */
	Base::Create(
		hwndParent,
		0, // extended window style
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN, // window style	// 2006.06.17 ryoji WS_CLIPCHILDREN 追加	// 2007.03.08 ryoji WS_VISIBLE 除去
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		0, // window width	// 2007.02.05 ryoji 100->0（半端なサイズで一瞬表示されるより見えないほうがいい）
		::GetSystemMetrics( SM_CYMENU ), // window height
		nullptr // handle to menu, or child-window identifier
	);

	m_hwndSizeBox = nullptr;
	if( m_bSizeBox ){
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 						/* no extended styles			*/
			WC_SCROLLBAR,				/* scroll bar control class		*/
			nullptr,						/* text for window title bar	*/
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,							/* horizontal position			*/
			0,							/* vertical position			*/
			200,						/* width of the scroll bar		*/
			CW_USEDEFAULT,				/* default height				*/
			GetHwnd(), 					/* handle of main window		*/
			(HMENU) nullptr,				/* no menu for a scroll bar 	*/
			GetAppInstance(),				/* instance owning this window	*/
			(LPVOID) nullptr				/* pointer not needed			*/
		);
	}

	/* ボタンの生成 */
	CreateButtons();

	Timer_ONOFF( true ); // 20060126 aroka
	OnTimer(GetHwnd(), IDT_FUNCWND);	// 初回更新

	return GetHwnd();
}

/* ウィンドウ クローズ */
void CFuncKeyWnd::Close( void )
{
	this->DestroyWindow();
}

#if 0//////////////////////////////////////////////////////////////
LRESULT CFuncKeyWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
//	if( NULL == GetHwnd() ){
//		return 0L;
//	}

	int		i;
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	switch ( uMsg ){

	case WM_TIMER:		return OnTimer( hwnd, uMsg, wParam, lParam );
	case WM_COMMAND:	return OnCommand( hwnd, uMsg, wParam, lParam );
	case WM_SIZE:		return OnSize( hwnd, uMsg, wParam, lParam );
	case WM_DESTROY:	return OnDestroy( hwnd, uMsg, wParam, lParam );

	default:
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}
#endif//////////////////////////////////////////////////////////////

/*! ボタンのサイズを計算 */
int CFuncKeyWnd::CalcButtonWidth(int cx)
{
	const auto nButtonNum = int(std::size(m_hwndButtonArr));

	const auto cxBorder = GetSystemMetrics(SM_CXBORDER);
	const auto cxEdge = GetSystemMetrics(SM_CXEDGE);

	return (cx - nButtonNum - ((nButtonNum + m_nButtonGroupNum - cxBorder) / m_nButtonGroupNum - cxBorder) * cxEdge * 6) / nButtonNum;
}

/*! ボタンの生成
	@date 2007.02.05 ryoji ボタンの水平位置・幅の設定処理を削除（OnSizeで再配置されるので不要）
*/
void CFuncKeyWnd::CreateButtons( void )
{
	RECT	rcParent;
	int		nButtonHeight;
	int		i;

	::GetWindowRect( GetHwnd(), &rcParent );
	nButtonHeight = rcParent.bottom - rcParent.top - 2;

	for( i = 0; i < int(std::size(m_nFuncCodeArr)); ++i ){
		m_nFuncCodeArr[i] = F_0;
	}

	for( i = 0; i < int(std::size(m_hwndButtonArr)); ++i ){
		m_hwndButtonArr[i] = ::CreateWindow(
			WC_BUTTON,							// predefined class
			L"",								// button text
			WS_VISIBLE | WS_CHILD | BS_LEFT,	// styles
			// Size and position values are given explicitly, because
			// the CW_USEDEFAULT constant gives zero values for buttons.
			0,					// starting x position
			0 + 1,				// starting y position
			0,					// button width
			nButtonHeight,		// button height
			GetHwnd(),				// parent window
			nullptr,				// No menu
			(HINSTANCE) GetWindowLongPtr(GetHwnd(), GWLP_HINSTANCE),	// Modified by KEITA for WIN64 2003.9.6
			nullptr				// pointer not needed
		);
		/* フォント変更 */
		SetWindowFont(m_hwndButtonArr[i], m_hFont, TRUE);
	}
	m_nCurrentKeyState = -1;
	return;
}

/*! サイズボックスの表示／非表示切り替え */
void CFuncKeyWnd::SizeBox_ONOFF( bool bSizeBox )
{
	if( m_bSizeBox == bSizeBox ){
		return;
	}

	const auto hWnd = GetHwnd();

	RECT rc{};
	::GetClientRect(hWnd, &rc);

	if( m_bSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = nullptr;
		m_bSizeBox = false;
	}else{
		const auto cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
		const auto cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

		m_hwndSizeBox = ::CreateWindowExW(
			0L, 						/* no extended styles			*/
			WC_SCROLLBAR,				/* scroll bar control class		*/
			nullptr,					/* text for window title bar	*/
			WS_CHILD | WS_VISIBLE | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			rc.right - cxVScroll,		/* horizontal position			*/
			rc.bottom - cyHScroll,		/* vertical position			*/
			cxVScroll,					/* width of the scroll bar		*/
			cyHScroll,					/* default height				*/
			hWnd,		 				/* handle of main window		*/
			(HMENU) nullptr,			/* no menu for a scroll bar 	*/
			GetAppInstance(),			/* instance owning this window	*/
			(LPVOID) nullptr			/* pointer not needed				*/
		);

		m_bSizeBox = true;
	}
	OnSize(hWnd, 0, rc.right, rc.bottom);
	return;
}

// タイマーの更新を開始／停止する。 20060126 aroka
// ファンクションキー表示はタイマーにより更新しているが、
// アプリのフォーカスが外れたときに親ウィンドウからON/OFFを
//	呼び出してもらうことにより、余計な負荷を停止したい。
void CFuncKeyWnd::Timer_ONOFF( bool bStart )
{
	if( nullptr != GetHwnd() ){
		if( bStart ){
			/* タイマーを起動 */
			if( 0 == ::SetTimer( GetHwnd(), IDT_FUNCWND, TIMER_TIMEOUT, nullptr ) ){
				WarningMessage(	GetHwnd(), LS(STR_ERR_DLGFUNCKEYWN1) );
			}
		} else {
			/* タイマーを削除 */
			::KillTimer( GetHwnd(), IDT_FUNCWND );
			m_nCurrentKeyState = -1;
		}
	}
	return;
}

/*!
 * @brief WM_DESTROYハンドラ
 *
 * WM_DESTROYはDestroyWindow関数によるウインドウ破棄中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CFuncKeyWnd::OnDestroy(HWND hWnd)
{
	/* サイズボックスを削除 */
	if (m_hwndSizeBox) {
		::DestroyWindow(m_hwndSizeBox);
		m_hwndSizeBox = nullptr;
	}

	/* タイマーを削除 */
	Timer_ONOFF(false);

	/* ボタンを削除 */
	for (auto& hWndButton : m_hwndButtonArr) {
		if (hWndButton) {
			::DestroyWindow(hWndButton);
			hWndButton = nullptr;
		}
	}

	//表示用フォントを削除する
	m_hFont = nullptr;

	Base::OnDestroy(hWnd);
}

/*!
 * @brief WM_SIZEハンドラ
 *
 * WM_SIZEはWM_WINDOWPOSCHANGEDの処理中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CFuncKeyWnd::OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	if (!hWnd || SIZE_MINIMIZED == state) {
		return;
	}

	if (m_hwndSizeBox) {
		const auto cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
		const auto cyHScroll = GetSystemMetrics(SM_CYHSCROLL);

		::SetWindowPos(
			m_hwndSizeBox,
			nullptr,
			cx - cxVScroll,
			cy - cyHScroll,
			cxVScroll,
			cyHScroll,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING
		);

		cx -= cxVScroll;
	}

	const auto nButtonNum = int(std::size(m_hwndButtonArr));

	const auto cxBorder = GetSystemMetrics(SM_CXBORDER);
	const auto cxEdge = GetSystemMetrics(SM_CXEDGE);
	const auto cyBorder = GetSystemMetrics(SM_CYBORDER);

	/* ボタンのサイズを計算 */
	const auto nButtonWidth = CalcButtonWidth(cx);

	const auto nButtonHeight = cy - cyBorder * 2;

	int nX = cxBorder;
	for (int i = 0; i < nButtonNum; ++i) {
		if( 0 < i  && 0 == ( i % m_nButtonGroupNum ) ){
			nX += cxEdge * 6;
		}

		::SetWindowPos(
			m_hwndButtonArr[i],
			nullptr,
			nX,
			cyBorder,
			nButtonWidth,
			nButtonHeight,
			SWP_NOZORDER | SWP_NOSENDCHANGING
		);

		nX += nButtonWidth + cxBorder;
	}

	::InvalidateRect(hWnd, nullptr, TRUE);
}

/*!
 * @brief WM_COMMANDハンドラ
 *
 * WM_COMMANDは子孫ウィンドウからポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CFuncKeyWnd::OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT notifyCode)
{
	UNREFERENCED_PARAMETER(id);
	UNREFERENCED_PARAMETER(notifyCode);

	const auto found = std::ranges::find_if(m_hwndButtonArr, [hWndCtl] (const auto& hWndButton) { return hWndButton == hWndCtl; });
	if (found == std::end(m_hwndButtonArr)) {
		return;
	}

	if (const auto i = std::distance(std::begin(m_hwndButtonArr), found); 0 != m_nFuncCodeArr[i]) {
		FORWARD_WM_COMMAND(GetParentHwnd(), m_nFuncCodeArr[i], hWnd, BN_CLICKED, ::SendMessageW);
	}

	::SetFocus(GetParentHwnd());
}

/*!
 * @brief WM_TIMERハンドラ
 *
 * WM_TIMERはSetTimer関数で作成したタイマーからポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CFuncKeyWnd::OnTimer(HWND hWnd, UINT id)
{
	UNREFERENCED_PARAMETER(id);

	if (!hWnd) {
		return;
	}

	if( ::GetActiveWindow() != GetParentHwnd() && m_nCurrentKeyState != -1 ) {	//	2002/06/02 MIK	// 2006.12.20 ryoji 初回更新は処理する
		return;
	}

	int			nIdx;
//	int			nFuncId;
	int			i;

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* ALT,Shift,Ctrlキーの状態が変化したか */
	if( nIdx != m_nCurrentKeyState ){
		m_nTimerCount = TIMER_CHECKFUNCENABLE + 1;

		/* ファンクションキーの機能名を取得 */
		for( i = 0; i < int(std::size(m_szFuncNameArr)); ++i ){
			// 2007.02.22 ryoji CKeyBind::GetFuncCode()を使う
			EFunctionCode	nFuncCode = CKeyBind::GetFuncCode(
					(WORD)(((VK_F1 + i) | ((WORD)((BYTE)(nIdx))) << 8)),
					m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
					m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
			);
			if( nFuncCode != m_nFuncCodeArr[i] ){
				m_nFuncCodeArr[i] = nFuncCode;
				if( 0 == m_nFuncCodeArr[i] ){
					m_szFuncNameArr[i][0] = L'\0';
				}else{
					//	Oct. 2, 2001 genta
					m_pcEditDoc->m_cFuncLookup.Funccode2Name(
						m_nFuncCodeArr[i],
						m_szFuncNameArr[i],
						_countof(m_szFuncNameArr[i]) - 1
					);
				}
				ApiWrap::Wnd_SetText( m_hwndButtonArr[i], m_szFuncNameArr[i] );
			}
		}
	}
	m_nTimerCount += TIMER_TIMEOUT;
	if( m_nTimerCount > TIMER_CHECKFUNCENABLE ||
		nIdx != m_nCurrentKeyState
	){
		m_nTimerCount = 0;
		/* 機能が利用可能か調べる */
		for( i = 0; i < int(std::size(m_szFuncNameArr)); ++i ){
			if( IsFuncEnable( (CEditDoc*)m_pcEditDoc, m_pShareData, m_nFuncCodeArr[i]  ) ){
				::EnableWindow( m_hwndButtonArr[i], TRUE );
			}else{
				::EnableWindow( m_hwndButtonArr[i], FALSE );
			}
		}
	}
	m_nCurrentKeyState = nIdx;
}
