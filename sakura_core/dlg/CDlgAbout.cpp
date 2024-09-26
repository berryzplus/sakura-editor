/*!	@file
	@brief バージョン情報ダイアログ

	@author Norio Nakatani
	@date	1998/3/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta, jepro
	Copyright (C) 2002, MIK, genta, aroka
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgAbout.h"
#include "uiparts/HandCursor.h"
#include "util/file.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h" // 2002/2/10 aroka 復帰
#include "version.h"
#include "apiwrap/StdApi.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"
#include "sakura.hh"
#include "config/system_constants.h"
#include "String_define.h"

// バージョン情報 CDlgAbout.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12900
	IDOK,					HIDOK_ABOUT,
	IDC_EDIT_ABOUT,			HIDC_ABOUT_EDIT_ABOUT,
//	IDC_STATIC_URL_UR,		12970,
//	IDC_STATIC_URL_ORG,		12971,
//	IDC_STATIC_UPDATE,		12972,
//	IDC_STATIC_VER,			12973,
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

//	From Here Feb. 7, 2002 genta
// 2006.01.17 Moca COMPILER_VERを追加
// 2010.04.15 Moca icc/dmcを追加しCPUを分離
#if defined(_M_IA64)
#  define TARGET_M_SUFFIX "_I64"
#elif defined(_M_AMD64)
#  define TARGET_M_SUFFIX "_A64"
#else
#  define TARGET_M_SUFFIX ""
#endif

#if defined(__BORLANDC__)
// borland c++
// http://docwiki.embarcadero.com/RADStudio/Rio/en/Predefined_Macros
// http://docwiki.embarcadero.com/RADStudio/Rio/en/Predefined_Macros#C.2B.2B_Compiler_Versions_in_Predefined_Macros
#  define COMPILER_TYPE "B"
#  define COMPILER_VER  __BORLANDC__ 
#elif defined(__GNUG__)
// __GNUG__ = (__GNUC__ && __cplusplus)
// GNU C++
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#  define COMPILER_TYPE "G"
#  define COMPILER_VER (__GNUC__ * 10000 + __GNUC_MINOR__  * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__INTEL_COMPILER)
// Intel Compiler
// https://software.intel.com/en-us/cpp-compiler-developer-guide-and-reference-additional-predefined-macros
#  define COMPILER_TYPE "I"
#  define COMPILER_VER __INTEL_COMPILER
#elif defined(__DMC__)
// Digital Mars C/C++
// https://digitalmars.com/ctg/predefined.html
#  define COMPILER_TYPE "D"
#  define COMPILER_VER __DMC__
#elif defined(_MSC_VER)
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
#  define COMPILER_TYPE "V"
#  define COMPILER_VER _MSC_VER
#else
// unknown
#  define COMPILER_TYPE "U"
#  define COMPILER_VER 0
#endif
//	To Here Feb. 7, 2002 genta

#define TARGET_STRING_MODEL "WP"

#ifdef _DEBUG
	#define TARGET_DEBUG_MODE "D"
#else
	#define TARGET_DEBUG_MODE "R"
#endif

#define TSTR_TARGET_MODE _T(TARGET_STRING_MODEL) _T(TARGET_DEBUG_MODE)

#ifdef _WIN32_WINDOWS
	#define MY_WIN32_WINDOWS _WIN32_WINDOWS
#else
	#define MY_WIN32_WINDOWS 0
#endif

#ifdef _WIN32_WINNT
	#define MY_WIN32_WINNT _WIN32_WINNT
#else
	#define MY_WIN32_WINNT 0
#endif

#if defined(CI_BUILD_URL)
#pragma message("CI_BUILD_URL: " CI_BUILD_URL)
#endif
#if defined(CI_BUILD_NUMBER_LABEL)
#pragma message("CI_BUILD_NUMBER_LABEL: " CI_BUILD_NUMBER_LABEL)
#endif

//	From Here Nov. 7, 2000 genta
/*!
	標準以外のメッセージを捕捉する
*/
INT_PTR CDlgAbout::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		// EDITも READONLY か DISABLEの場合 WM_CTLCOLORSTATIC になります
		if( (HWND)lParam == GetDlgItem(hWnd, IDC_EDIT_ABOUT) ){
			::SetTextColor( (HDC)wParam, RGB( 102, 102, 102 ) );
		} else {
			::SetTextColor( (HDC)wParam, RGB( 0, 0, 0 ) );
        }
		return (INT_PTR)GetStockObject( WHITE_BRUSH );
	}
	return result;
}
//	To Here Nov. 7, 2000 genta

/* モーダルダイアログの表示 */
int CDlgAbout::DoModal( HINSTANCE hInstance, HWND hwndParent )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_ABOUT, (LPARAM)NULL );
}

/*! 初期化処理
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
	@date 2011.04.10 nasukoji	各国語メッセージリソース対応
	@date 2013.04.07 novice svn revision 情報追加
*/
BOOL CDlgAbout::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	WCHAR			szFile[_MAX_PATH];

	/* この実行ファイルの情報 */
	::GetModuleFileName( NULL, szFile, _countof( szFile ) );
	
	/* バージョン情報 */
	//	Nov. 6, 2000 genta	Unofficial Releaseのバージョンとして設定
	//	Jun. 8, 2001 genta	GPL化に伴い、OfficialなReleaseとしての道を歩み始める
	//	Feb. 7, 2002 genta コンパイラ情報追加
	//	2004.05.13 Moca バージョン番号は、プロセスごとに取得する
	//	2010.04.15 Moca コンパイラ情報を分離/WINヘッダー,N_SHAREDATA_VERSION追加

	// 以下の形式で出力
	//サクラエディタ開発版(64bitデバッグ) Ver. 2.4.1.1234 GHA (xxxxxxxx)
	//(GitURL https://github.com/sakura/sakura-editor.git)
	//
	//      Share Ver: 96
	//      Compile Info: V 1400  WR WIN600/I601/C000/N600
	//      Last Modified: 1999/9/9 00:00:00
	//      (あればSKR_PATCH_INFOの文字列がそのまま表示)
	CNativeW cmemMsg;

	// 1行目
	// バージョン情報
	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo( NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS );
	
	cmemMsg.AppendStringF(
		L"%s Ver. %d.%d.%d.%d " LTEXT(BUILD_ENV_NAME) LTEXT(VERSION_HASH) L"\r\n",
		LS(STR_GSTR_APPNAME),
		HIWORD(dwVersionMS), LOWORD(dwVersionMS), HIWORD(dwVersionLS), LOWORD(dwVersionLS) // e.g. {2, 3, 2, 0}
	);

	// 2行目
#ifdef GIT_COMMIT_HASH
	cmemMsg.AppendString( L"(GitHash " _T(GIT_COMMIT_HASH) L")\r\n" );
#endif

	// 3行目
#ifdef GIT_REMOTE_ORIGIN_URL
	cmemMsg.AppendString( L"(GitURL " _T(GIT_REMOTE_ORIGIN_URL) L")\r\n");
#endif

	// 段落区切り
	cmemMsg.AppendString( L"\r\n" );

	// コンパイル情報
	cmemMsg.AppendStringF(
		L"      Compile Info: " _T(COMPILER_TYPE) _T(TARGET_M_SUFFIX) L"%d " TSTR_TARGET_MODE L" WIN%03x/I%03x/C%03x/N%03x\r\n",
		COMPILER_VER, WINVER, _WIN32_IE, MY_WIN32_WINDOWS, MY_WIN32_WINNT
	);

	// 更新日情報
	//	Oct. 22, 2005 genta タイムスタンプ取得の共通関数利用
	CFileTime cFileTime;
	GetLastWriteTimestamp( szFile, &cFileTime );
	cmemMsg.AppendStringF(
		L"      Last Modified: %d/%d/%d %02d:%02d:%02d\r\n",
		cFileTime->wYear,
		cFileTime->wMonth,
		cFileTime->wDay,
		cFileTime->wHour,
		cFileTime->wMinute,
		cFileTime->wSecond
	);

	// パッチの情報をコンパイル時に渡せるようにする
#ifdef SKR_PATCH_INFO
	cmemMsg.AppendString( L"      " );
	const WCHAR szPatchInfo[] = SKR_PATCH_INFO;
	size_t patchInfoLen = _countof(szPatchInfo) - 1;
	cmemMsg.AppendString( szPatchInfo, t_min(80, patchInfoLen) );
#endif
	cmemMsg.AppendString( L"\r\n");

	::DlgItem_SetText( GetHwnd(), IDC_EDIT_VER, cmemMsg.GetStringPtr() );

	//	From Here Jun. 8, 2001 genta
	//	Edit Boxにメッセージを追加する．
	// 2011.06.01 nasukoji	各国語メッセージリソース対応
	LPCWSTR pszDesc = LS( IDS_ABOUT_DESCRIPTION );
	WCHAR szMsg[2048];
	if( pszDesc[0] != '\0' ) {
		wcsncpy( szMsg, pszDesc, _countof(szMsg) - 1 );
		szMsg[_countof(szMsg) - 1] = 0;
		::DlgItem_SetText( GetHwnd(), IDC_EDIT_ABOUT, szMsg );
	}
	//	To Here Jun. 8, 2001 genta

	//	From Here Dec. 2, 2002 genta
	//	アイコンをカスタマイズアイコンに合わせる
	HICON hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	HWND hIconWnd = GetItemHwnd( IDC_STATIC_MYICON );
	
	if( hIconWnd != NULL && hIcon != NULL ){
		StCtl_SetIcon( hIconWnd, hIcon );
	}
	//	To Here Dec. 2, 2002 genta

	/* 基底クラスメンバ */
	(void)CDialog::OnInitDialog( GetHwnd(), wParam, lParam );

	// URLウィンドウをサブクラス化する
	m_UrlUrWnd.SetSubclassWindow( GetItemHwnd( IDC_STATIC_URL_UR ) );
#ifdef GIT_REMOTE_ORIGIN_URL
	m_UrlGitWnd.SetSubclassWindow( GetItemHwnd( IDC_STATIC_URL_GIT ) );
#endif
#ifdef CI_BUILD_NUMBER_LABEL
	m_UrlBuildLinkWnd.SetSubclassWindow( GetItemHwnd( IDC_STATIC_URL_CI_BUILD ) );
#endif
#if defined( GITHUB_COMMIT_URL )
	m_UrlGitHubCommitWnd.SetSubclassWindow( GetItemHwnd( IDC_STATIC_URL_GITHUB_COMMIT ) );
#endif
#if defined( GITHUB_PR_HEAD_URL )
	m_UrlGitHubPRWnd.SetSubclassWindow( GetItemHwnd( IDC_STATIC_URL_GITHUB_PR ) );
#endif

	//	Oct. 22, 2005 genta 原作者ホームページが無くなったので削除
	//m_UrlOrgWnd.SubclassWindow( GetItemHwnd(IDC_STATIC_URL_ORG ) );

	/* デフォルトでは一番最初のタブオーダーの要素になるので明示的に OK ボタンにフォーカスを合わせる */
	::SetFocus(GetItemHwnd(IDOK));

	/*
		SetFocus() の効果を有効にするために FALSE を返す
		参考: https://msdn.microsoft.com/ja-jp/library/fwz35s59.aspx
	*/
	return FALSE;
}

BOOL CDlgAbout::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_COPY:
		{
			HWND hwndEditVer = GetItemHwnd( IDC_EDIT_VER );
	 		EditCtl_SetSel( hwndEditVer, 0, -1); 
	 		SendMessage( hwndEditVer, WM_COPY, 0, 0 );
	 		EditCtl_SetSel( hwndEditVer, -1, 0); 
 		}
		return TRUE;
	}
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgAbout::OnStnClicked( int wID )
{
	switch( wID ){
	//	2006.07.27 genta 原作者連絡先のボタンを削除 (ヘルプから削除されているため)
	case IDC_STATIC_URL_UR:
	case IDC_STATIC_URL_GIT:
//	case IDC_STATIC_URL_ORG:	del 2008/7/4 Uchi
		//	Web Browserの起動
		{
			std::wstring url;
			ApiWrap::DlgItem_GetText(GetHwnd(), wID, url);
			OpenWithBrowser(GetHwnd(), url);
			return TRUE;
		}
	case IDC_STATIC_URL_CI_BUILD:
		{
#if defined(CI_BUILD_URL)
			OpenWithBrowser(GetHwnd(), _T(CI_BUILD_URL));
#elif defined(GIT_REMOTE_ORIGIN_URL)
			OpenWithBrowser(GetHwnd(), _T(GIT_REMOTE_ORIGIN_URL));
#endif
			return TRUE;
		}
	case IDC_STATIC_URL_GITHUB_COMMIT:
#if defined(GITHUB_COMMIT_URL)
		OpenWithBrowser(GetHwnd(), _T(GITHUB_COMMIT_URL));
#endif
		return TRUE;
	case IDC_STATIC_URL_GITHUB_PR:
#if defined(GITHUB_PR_HEAD_URL)
		OpenWithBrowser(GetHwnd(), _T(GITHUB_PR_HEAD_URL));
#endif
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnStnClicked( wID );
}

//@@@ 2002.01.18 add start
LPVOID CDlgAbout::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

bool CUrlWnd::Attach(HWND hWnd)
{
    if (__super::Attach(hWnd)) {
		const auto hFont = HFONT(SendMessageW(hWnd, WM_GETFONT, 0, 0));

		LOGFONT lf = {};
		GetObjectW(hFont, sizeof(lf), &lf);

		// 下線付きフォントに変更する
		lf.lfUnderline = TRUE;
		m_hFont = CreateFontIndirectW(&lf);
		m_Font.reset(m_hFont);

		if (m_hFont) {
			DispatchEvent(hWnd, WM_SETFONT, WPARAM(m_hFont), FALSE);
		}
    }

	return true;
}

void CUrlWnd::Detach(HWND hWnd)
{
	KillTimer(hWnd, 1);

	__super::Detach(hWnd);

	m_Font.reset();
	m_hFont = nullptr;
	m_bHilighted = FALSE;
	m_BrushHilightedBackground.reset();
}

BOOL CUrlWnd::SetSubclassWindow( HWND hWnd )
{
	// STATICウィンドウをサブクラス化する
	// 元のSTATICは WS_TABSTOP, SS_NOTIFY スタイルのものを使用すること
	return Attach(hWnd);
}

/*!
 * CUrlWndのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CUrlWnd::DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto pUrlWnd = this;

	HDC hdc;
	POINT pt;
	RECT rc;

	switch (uMsg) {
// clang-format off
	HANDLE_MSG(hWnd, WM_SETTEXT,                        OnSetText);
	HANDLE_MSG(hWnd, WM_SETFONT,                        OnSetFont);
// clang-format on

	case WM_SETCURSOR:
		// カーソル形状変更
		SetHandCursor();		// Hand Cursorを設定 2013/1/29 Uchi
		return (LRESULT)0;

	case WM_LBUTTONDOWN:
		// キーボードフォーカスを自分に当てる
		SendMessageAny( GetParent(hWnd), WM_NEXTDLGCTL, (WPARAM)hWnd, (LPARAM)1 );
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		// 再描画
		InvalidateRect( hWnd, NULL, TRUE );
		UpdateWindow( hWnd );
		break;

	case WM_GETDLGCODE:
		// デフォルトプッシュボタンのように振舞う（Enterキーの有効化）
		// 方向キーは無効化（IEのバージョン情報ダイアログと同様）
		return DLGC_DEFPUSHBUTTON | DLGC_WANTARROWS;

	case WM_MOUSEMOVE:
		// カーソルがウィンドウ内に入ったらタイマー起動
		// ウィンドウ外に出たらタイマー削除
		// 各タイミングで再描画
		BOOL bHilighted;
		pt.x = LOWORD( lParam );
		pt.y = HIWORD( lParam );
		GetClientRect( hWnd, &rc );
		bHilighted = PtInRect( &rc, pt );
		if( bHilighted != pUrlWnd->m_bHilighted ){
			pUrlWnd->m_bHilighted = bHilighted;
			InvalidateRect( hWnd, NULL, TRUE );
			if( pUrlWnd->m_bHilighted )
				SetTimer( hWnd, 1, 200, NULL );
			else
				KillTimer( hWnd, 1 );
		}
		break;

	case WM_TIMER:
		// カーソルがウィンドウ外にある場合にも WM_MOUSEMOVE を送る
		GetCursorPos( &pt );
		ScreenToClient( hWnd, &pt );
		GetClientRect( hWnd, &rc );
		if( !PtInRect( &rc, pt ) )
			SendMessageAny( hWnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
		break;

	case WM_PAINT:
		// ウィンドウの描画
		PAINTSTRUCT ps;
		hdc = BeginPaint( hWnd, &ps );

		// 現在のクライアント矩形、テキスト、フォントを取得する
		GetClientRect( hWnd, &rc );

		// テキスト描画
		if (const auto text = GetText();
			text.size())
		{
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, m_bHilighted? RGB(0x84, 0, 0) : RGB(0, 0, 0xff));
			auto hFontOld = apiwrap::gdi::select_object(hdc, m_Font.get());
			TextOutW(hdc, DpiScaleX(2), 0, text.data(), int(text.length()));
		}

		// フォーカス枠描画
		if( GetFocus() == hWnd )
			DrawFocusRect( hdc, &rc );

		EndPaint( hWnd, &ps );
		return (LRESULT)0;
	case WM_ERASEBKGND:
		hdc = (HDC)wParam;
		GetClientRect( hWnd, &rc );

		// 背景描画
		if( pUrlWnd->m_bHilighted ){
			// ハイライト時背景描画
			HBRUSH brush = ::CreateSolidBrush( RGB( 0xff, 0xff, 0 ) );
			HGDIOBJ brushOld = ::SelectObject( hdc, brush );
			::PatBlt( hdc, rc.left, rc.top, rc.right, rc.bottom, PATCOPY );
			::SelectObject( hdc, brushOld );
			::DeleteObject( brush );
		}else{
			// 親にWM_CTLCOLORSTATICを送って背景ブラシを取得し、背景描画する
			const auto hBrush = HBRUSH(SendMessageW(GetParent(hWnd), WM_CTLCOLORSTATIC, wParam, LPARAM(hWnd)));
			auto hBrushOld = apiwrap::gdi::select_object(hdc, hBrush);
			PatBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, PATCOPY);
		}
		return (LRESULT)1;

	default:
		break;
	}

	return __super::DispatchEvent(hWnd, uMsg, wParam, lParam);
}
//@@@ 2002.01.18 add end

/*!
 * WM_SETTEXTハンドラ
 *
 * @sa https://docs.microsoft.com/en-us/windows/desktop/winmsg/wm-settext
 */
void CUrlWnd::OnSetText(HWND hWnd, _In_opt_z_ LPCWSTR pchText)
{
	WCHAR chDummy = 0;
	pchText = pchText ? pchText : &chDummy;

	// 標準のメッセージハンドラに処理させる
	DefWindowProcW(hWnd, WM_SETTEXT, 0, LPARAM(pchText));

	const auto cchText = int(auto_strlen(pchText));

	// サイズを調整のためにDCを取得
	const auto hDC = GetDC(hWnd);

	//終了時にReleaseDCを呼び出すよう設定
	const auto releaseDC = [hWnd](const HDC h) { ::ReleaseDC(hWnd, h); };
	using windowDcReleaser = std::unique_ptr<std::remove_pointer_t<HDC>, decltype(releaseDC)>;
	windowDcReleaser dcReleaser(hDC, releaseDC);

	auto hFontOld = apiwrap::gdi::select_object(hDC, m_Font.get());

	// DrawText関数を使ってサイズを計測する
	// ※この処理は実際には描かない
	if (CMyRect rcText;
		DrawTextW(hDC, pchText, cchText, &rcText, DT_CALCRECT))
	{
		// マージン用にシステム設定値を取得する。
		// ※ユーザーが変えられる値なので毎回取りに行く（EDGE = 2px on 96dpi）
		const auto cxEdge = GetSystemMetrics(SM_CXEDGE);
		const auto cyEdge = GetSystemMetrics(SM_CYEDGE);

		// 計測結果のRECT構造体をSIZE構造体に読み替え、マージンを付加する
		SIZE size;
		size.cx = cxEdge + rcText.Width()  + cxEdge;
		size.cy = cyEdge + rcText.Height() + cyEdge;

		// マージン込みのサイズをウインドウに反映する
		SetWindowPos(hWnd, nullptr, 0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}
}

/*!
 * WM_SETFONTハンドラ
 */
void CUrlWnd::OnSetFont(HWND hWnd, HFONT hFont, bool fRedraw)
{
	// fRedrawを無視して、常に再描画する
	UNREFERENCED_PARAMETER(fRedraw);

	// 標準のメッセージハンドラに処理させる
	DefWindowProcW(hWnd, WM_SETFONT, WPARAM(hFont), LPARAM(false));

	// 設定されているテキストを取得する
	const auto text = GetText();
	DispatchEvent(hWnd, WM_SETTEXT, 0, LPARAM(text.data()));
}
