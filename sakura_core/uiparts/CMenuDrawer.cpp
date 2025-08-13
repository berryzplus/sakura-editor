/*!	@file
	@brief メニュー管理＆表示

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, Jepro
	Copyright (C) 2001, jepro, MIK, Misaka, YAZAKI, hor, genta
	Copyright (C) 2002, MIK, genta, YAZAKI, ai, Moca, hor, aroka
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, Kazika, genta, Moca, isearch
	Copyright (C) 2005, genta, MIK, aroka
	Copyright (C) 2006, aroka, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CMenuDrawer.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CSplitBoxWnd.h"
#include "CImageListMgr.h"
#include "func/CKeyBind.h"
#include "uiparts/CGraphics.h"
#include "util/window.h"

// メニューアイコンの背景をボタンの色にする
#define DRAW_MENU_ICON_BACKGROUND_3DFACE

// メニューの選択色を淡くする
#define DRAW_MENU_SELECTION_LIGHT

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CMenuDrawer::CMenuDrawer()
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。	/* ツールバーのボタン TBBUTTON構造体 */
	/* ツールバーのボタン TBBUTTON構造体 */
	/*
	typedef struct _TBBUTTON {
		int iBitmap;	// ボタン イメージの 0 から始まるインデックス
		int idCommand;	// ボタンが押されたときに送られるコマンド
		BYTE fsState;	// ボタンの状態--以下を参照
		BYTE fsStyle;	// ボタン スタイル--以下を参照
		DWORD dwData;	// アプリケーション-定義された値
		int iString;	// ボタンのラベル文字列の 0 から始まるインデックス
	} TBBUTTON;
	*/
//	キーワード：アイコン順序(アイコンインデックス)
//	Sept. 16, 2000 Jepro note: アイコン登録メニュー
//	以下の登録はツールバーだけでなくアイコンをもつすべてのメニューで利用されている
//	数字はビットマップリソースのIDB_MYTOOLに登録されているアイコンの先頭からの順番のようである
//	アイコンをもっと登録できるように横幅を16dotsx218=2048dotsに拡大
//	縦も15dotsから16dotsにして「プリンター」アイコンや「ヘルプ1」の、下が欠けている部分を補ったが15dotsまでしか表示されないらしく効果なし
//	→
//	Sept. 17, 2000 縦16dot目を表示できるようにした
//	修正したファイルにはJEPRO_16thdotとコメントしてあるのでもし間違っていたらそれをキーワードにして検索してください(Sept. 28, 2000現在 6箇所変更)
//	IDB_MYTOOLの16dot目に見やすいように横16dotsづつの区切りになる目印を付けた
//
//	Sept. 16, 2000 見やすいように横に20個(あるいは32個)づつに配列しようとしたが配列構造を変えなければうまく格納できないので
//	それを解決するのが先決(→げんた氏改修版ur3β13で解決)
//
//	Sept. 16, 2000 JEPRO できるだけ系ごとに集まるように順番を大幅に入れ替えた  それに伴いCShareData.cppで設定している初期設定値も変更
//	Oct. 22, 2000 JEPRO アイコンのビットマップリソースの2次元配置が可能になったため根本的に配置転換した
//	・配置の基本は「コマンド一覧」に入っている機能(コマンド)順	なお「コマンド一覧」自体は「メニューバー」の順におおよそ準拠している
//	・アイコンビットマップファイルには横32個X15段ある(2010.06.26 13段から拡張)
//	・互換性と新コマンド追加の両立の都合で飛び地あり
//	・メニューに属する系および各系の段との関係は次の通り(2012.03.10 現在)：
//		ファイル----- ファイル操作系	(1段目32個: 1-32)
//		編集--------- 編集系			(2段目32個: 33-64)
//		移動--------- カーソル移動系	(3段目32個: 65-96)
//		選択--------- 選択系			(4段目32個: 97-128)
//					+ 矩形選択系		(5段目32個: 129-160) //(注. 矩形選択系のほとんどは未実装)
//					+ クリップボード系	(6段目24個: 161-184)
//			★挿入系					(6段目残りの8個: 185-192)
//		変換--------- 変換系			(7段目32個: 193-224)
//		検索--------- 検索系			(8段目32個: 225-256)
//		ツール------- モード切り替え系	(9段目4個: 257-260)
//					+ 設定系			(9段目次の16個: 261-276)
//					+ マクロ系			(9段目最後の11個: 277-287)
//					+ 外部マクロ		(12段目32個: 353-384/13段目19個: 385-403)
//					+ カスタムメニュー	(10段目25個: 289-313)
//		ウィンドウ--- ウィンドウ系		(11段目22個: 321-342)
//					+ タブ系			(10段目残りの7個: 314-320/9段目最期の1個: 288)
//		ヘルプ------- 支援				(11段目残りの10個: 343-352)
//	注1.「挿入系」はメニューでは「編集」に入っている
//	注2.「コマンド一覧」に入ってないコマンドもわかっている範囲で位置予約にしておいた
//  注3. F_DISABLE は未定義用(ダミーとしても使う)
//	注4. ユーザー用に確保された場所は特にないので各段の空いている後ろの方を使ってください。
//	注5. アイコンビットマップの有効段数は、CImageListMgr の MAX_Y です。

	const auto& tbd = CImageListMgr::gm_toolIcons;

	// アイコン番号
	assert_warning(tbd[TOOLBAR_ICON_MACRO_INTERNAL] == F_MACRO_EXTRA);
	assert_warning(tbd[TOOLBAR_ICON_PLUGCOMMAND_DEFAULT] == F_PLUGCOMMAND);

	// コマンド番号
	assert_warning(tbd[TOOLBAR_BUTTON_F_TOOLBARWRAP] == F_TOOLBARWRAP);

	m_tbMyButton.reserve(std::size(tbd));

	for (size_t i = 0; i < std::size(tbd); ++i ) {
		auto funcCode = int(tbd[i]);
		auto iBitmap = int(i);

		BYTE fsState = 0;
		BYTE fsStyle = TBSTYLE_BUTTON;

		switch (funcCode)
		{
		case F_TOOLBARWRAP:	// ツールバー改行用の仮想ボタン（実際は表示されない）
			funcCode = F_MENU_NOT_USED_FIRST;
			fsState = TBSTATE_WRAP;
			[[fallthrough]];

		case F_SEPARATOR:	//セパレーター
			fsStyle = TBSTYLE_SEP;
			break;

		case F_FILEOPEN_DROPDOWN:
			fsStyle = TBSTYLE_DROPDOWN;	//ドロップダウン
			break;

		case F_SEARCH_BOX:
			fsStyle = TBSTYLE_COMBOBOX;	//コンボボックス
			break;

		default:
			break;
		}

		// ダミーコードのアイコンは未定義とする
		if (funcCode <= F_DUMMY_MAX_CODE) {
			iBitmap = -1;
		}

		// 割当のないボタンは無効にする
		if (F_0 == tbd[i]) {
			fsState += TBSTATE_ENABLED;
		}

		m_tbMyButton.emplace_back(
			iBitmap,	// iBitmap
			funcCode,	// idCommand
			fsState,	// fsState
			fsStyle		// fsStyle
		);
	}

	m_nMyButtonFixSize = int(m_tbMyButton.size());
	m_nMyButtonNum = int(m_tbMyButton.size());
}

CMenuDrawer::~CMenuDrawer()
{
	if( nullptr != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = nullptr;
	}
	DeleteCompDC();
	return;
}

void CMenuDrawer::Create( HINSTANCE hInstance, HWND hWndOwner, CImageListMgr* pcIcons )
{
	m_hInstance = hInstance;
	m_hWndOwner = hWndOwner;
	m_pcIcons = pcIcons;

	return;
}

void CMenuDrawer::ResetContents( void )
{
	LOGFONT	lf;
	m_menuItems.clear();

	NONCLIENTMETRICS	ncm;
	memset_raw(&ncm, 0, sizeof(ncm));

	// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する	// 2007.12.21 ryoji
	ncm.cbSize = CCSIZEOF_STRUCT( NONCLIENTMETRICS, lfMessageFont );
	::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0 );

	if( nullptr != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = nullptr;
	}
	lf = ncm.lfMenuFont;
	m_hFontMenu = ::CreateFontIndirect( &lf );
	m_nMenuFontHeight = lf.lfHeight;
	if( m_nMenuFontHeight < 0 ){
		m_nMenuFontHeight = -m_nMenuFontHeight;
	}else{
		// ポイント(1/72インチ)をピクセルへ
		m_nMenuFontHeight = DpiScaleY(m_nMenuFontHeight);
		if( -1 == m_nMenuFontHeight ){
			m_nMenuFontHeight = lf.lfHeight;
		}
	}
	m_nMenuHeight = m_nMenuFontHeight + DpiScaleY(4); // margin
	if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
		// 最低アイコン分の高さを確保
		if( 20 > m_nMenuHeight ){
			m_nMenuHeight = 20;
		}
	}

//@@@ 2002.01.03 YAZAKI 不使用のため
//	m_nMaxTab = 0;
//	m_nMaxTabLen = 0;
	return;
}

/* メニュー項目を追加 */
void CMenuDrawer::MyAppendMenu(
	HMENU			hMenu,
	int				nFlag,
	UINT_PTR		nFuncId,
	const WCHAR*	pszLabel,
	const WCHAR*	pszKey,			// 2010/5/18 Uchi
	BOOL			bAddKeyStr,
	int				nForceIconId	//お気に入り	//@@@ 2003.04.08 MIK
)
{
	WCHAR		szLabel[_MAX_PATH * 2+ 30];
	WCHAR		szKey[10];
	int			nFlagAdd = 0;

	if( nForceIconId == -1 ) nForceIconId = nFuncId;	//お気に入り	//@@@ 2003.04.08 MIK

	szLabel[0] = L'\0';
	if( nullptr != pszLabel ){
		wcsncpy( szLabel, pszLabel, _countof( szLabel ) - 1 );
		szLabel[ _countof( szLabel ) - 1 ] = L'\0';
	}
	wcscpy( szKey, pszKey); 
	if( nFuncId != 0 ){
		/* メニューラベルの作成 */
		CKeyBind::GetMenuLabel(
			m_hInstance,
			m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
			m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr,
			nFuncId,
			szLabel,
			szKey,
			bAddKeyStr,
			_countof(szLabel)
		 );

		/* アイコン用ビットマップを持つものは、オーナードロウにする */
		{
			MyMenuItemInfo item;
			item.m_nBitmapIdx = -1;
			item.m_nFuncId = nFuncId;
			item.m_cmemLabel.SetString( szLabel );
			// メニュー項目をオーナー描画にして、アイコンを表示する
			// 2010.03.29 アクセスキーの分を詰めるためいつもオーナードローにする。ただしVista未満限定
			// Vista以上ではメニューもテーマが適用されるので、オーナードローにすると見た目がXP風になってしまう。
			if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
				nFlagAdd = MF_OWNERDRAW;
			}
			/* 機能のビットマップの情報を覚えておく */
			item.m_nBitmapIdx = GetIconIdByFuncId( nForceIconId );
			m_menuItems.push_back( item );
		}
	}else{
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
		// セパレータかサブメニュー
		if( nFlag & (MF_SEPARATOR | MF_POPUP) ){
			if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
					nFlagAdd = MF_OWNERDRAW;
			}
		}
#endif
	}

	// メニュー項目に関する情報を設定します。
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = 0;
	if( MF_OWNERDRAW	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_OWNERDRAW;
	if( MF_SEPARATOR	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_SEPARATOR;
	if( MF_STRING		& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_STRING;
	if( MF_MENUBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBREAK;
	if( MF_MENUBARBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBARBREAK;

	mii.fState = 0;
	if( MF_GRAYED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_GRAYED;
	if( MF_CHECKED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_CHECKED;

	mii.wID = nFuncId;
	mii.hSubMenu = (nFlag&MF_POPUP)?((HMENU)nFuncId):nullptr;
	mii.hbmpChecked = nullptr;
	mii.hbmpUnchecked = nullptr;
	mii.dwItemData = (ULONG_PTR)this;
	mii.dwTypeData = szLabel;
	mii.cch = 0;

	// メニュー内の指定された位置に、新しいメニュー項目を挿入します。
	::InsertMenuItem( hMenu, 0xFFFFFFFF, TRUE, &mii );
	return;
}

/*
	ツールバー番号をボタン配列のindexに変換する
*/
inline int CMenuDrawer::ToolbarNoToIndex( int nToolbarNo ) const
{
	if( nToolbarNo < 0 ){
		return -1;
	}
	// 固定アクセス分のみ直接番号でアクセスさせる。m_nMyButtonNum は使わない
	if( 0 <= nToolbarNo && nToolbarNo < m_nMyButtonFixSize ){
		return nToolbarNo;
	}
	int nFuncID = nToolbarNo;
	return FindIndexFromCommandId( nFuncID, false );
}
 
/*
	ツールバー番号からアイコン番号を取得
*/
inline int CMenuDrawer::GetIconIdByFuncId( int nFuncID ) const
{
	int index = FindIndexFromCommandId( nFuncID, false );
	if( index < 0 ){
		return -1;
	}
	return m_tbMyButton[index].iBitmap;
}

/*! メニューアイテムの描画サイズを計算
	@param pnItemHeight [out] 高さ。いつも高さを返す
	@retval 0  機能がない場合
	@retval 1 <= val 機能のメニュー幅/セパレータの場合はダミーの値
*/
int CMenuDrawer::MeasureItem( int nFuncID, int* pnItemHeight )
{
	// pixel数をベタ書きするとHighDPI環境でずれるのでシステム値を取得して使う
	const int cxBorder = ::GetSystemMetrics(SM_CXBORDER);
	const int cyBorder = ::GetSystemMetrics(SM_CYBORDER);
	const int cxEdge = ::GetSystemMetrics(SM_CXEDGE);
	const int cyEdge = ::GetSystemMetrics(SM_CYEDGE);
	const int cxFrame = ::GetSystemMetrics(SM_CXFRAME);
	const int cyFrame = ::GetSystemMetrics(SM_CYFRAME);
	const int cxSmIcon = ::GetSystemMetrics(SM_CXSMICON);
	const int cySmIcon = ::GetSystemMetrics(SM_CYSMICON);

	const WCHAR* pszLabel;
	CMyRect rc, rcSp;
	HDC hdc;
	HFONT hFontOld;

	if( F_0 == nFuncID ){ // F_0, なぜか F_SEPARATOR ではない
		// セパレータ。フォントの方の通常項目の半分の高さ
		*pnItemHeight = m_nMenuFontHeight / 2;
		return 30; // ダミーの幅
	}else if( nullptr == ( pszLabel = GetLabel( nFuncID ) ) ){
		*pnItemHeight = m_nMenuHeight;
		return 0;
	}
	//正常な高さは幅と一緒に決める

	hdc = ::GetDC( m_hWndOwner );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );
	// DT_EXPANDTABSをやめる
	::DrawText( hdc, pszLabel, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT );
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWndOwner, hdc );

//	*pnItemHeight = 20;
//	*pnItemHeight = 2 + 15 + 1;
	//@@@ 2002.2.2 YAZAKI Windowsの設定でメニューのフォントを大きくすると表示が崩れる問題に対処

	// インデント + テキスト幅 + アクセスキー隙間
	int nMenuWidth = cxSmIcon / 4 + rc.Width() + cxSmIcon / 2;
	if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
		// アイコンと枠 + 縦線隙間 + 縦線
		// 2+[2+16+2]+2 + 2+2 + 1
		nMenuWidth += cxSmIcon + cxEdge * 6 + cxBorder;
	}else{
		// WM_MEASUREITEMで報告するメニュー幅より実際の幅は1文字分相当位広いので、その分は加えない
		nMenuWidth += ::GetSystemMetrics(SM_CXMENUCHECK) + 2 + 2;
	}
	// アイコンと枠 or フォント高さと太枠
	// 2+[2+16+2]+2 or 2+9+2
	*pnItemHeight = std::max(cySmIcon + cyEdge * 4, m_nMenuHeight + cyEdge * 2);
	return nMenuWidth;
}

/*! メニューアイテム描画
	@date 2001.12.21 YAZAKI デバッグモードでもメニューを選択したらハイライト。
	@date 2003.08.27 Moca システムカラーのブラシはCreateSolidBrushをやめGetSysColorBrushに
	@date 2010.07.24 Moca アイコン部分をボタン色にしてフラット表示にするなどの変更
		大きいフォント、黒背景対応
*/
void CMenuDrawer::DrawItem( DRAWITEMSTRUCT* lpdis )
{
	// pixel数をベタ書きするとHighDPI環境でずれるのでシステム値を取得して使う
	const int cxBorder = ::GetSystemMetrics(SM_CXBORDER);
	const int cyBorder = ::GetSystemMetrics(SM_CYBORDER);
	const int cxEdge = ::GetSystemMetrics(SM_CXEDGE);
	const int cyEdge = ::GetSystemMetrics(SM_CYEDGE);
	const int cxFrame = ::GetSystemMetrics(SM_CXFRAME);
	const int cyFrame = ::GetSystemMetrics(SM_CYFRAME);
	const int cxSmIcon = ::GetSystemMetrics(SM_CXSMICON);
	const int cySmIcon = ::GetSystemMetrics(SM_CYSMICON);

	CMyRect rcItem( lpdis->rcItem );

	const bool bMenuIconDraw = !!m_pShareData->m_Common.m_sWindow.m_bMenuIcon;
	const int nCxCheck = ::GetSystemMetrics(SM_CXMENUCHECK);
	const int nCyCheck = ::GetSystemMetrics(SM_CYMENUCHECK);

	// アイコンとテキストの間の縦線の位置
	const int nIndentLeft = bMenuIconDraw
		? cxSmIcon + cxEdge * 6 + cxBorder
		: cxEdge * 2 + nCxCheck;

	// サブメニューの|＞の分は必要 最低8ぐらい
	const int nIndentRight = cxSmIcon / 2;

	// 2010.07.24 Moca アイコンを描くときにチラつくので、バックサーフェスを使う
	const bool bBackSurface = bMenuIconDraw;
	const int nTargetWidth  = lpdis->rcItem.right - lpdis->rcItem.left;
	const int nTargetHeight = lpdis->rcItem.bottom - lpdis->rcItem.top;
	HDC hdcOrg = nullptr;
	HDC hdc = nullptr;
	if( bBackSurface ){
		hdcOrg = lpdis->hDC;
		if( m_hCompDC && nTargetWidth <= m_nCompBitmapWidth && nTargetHeight <= m_nCompBitmapHeight ){
			hdc = m_hCompDC;
		}else{
			if( m_hCompDC ){
				DeleteCompDC();
			}
			hdc = m_hCompDC  = ::CreateCompatibleDC( hdcOrg );
			m_hCompBitmap    = ::CreateCompatibleBitmap( hdcOrg, nTargetWidth + 20, nTargetHeight + 4 );
			m_hCompBitmapOld = (HBITMAP)::SelectObject( hdc, m_hCompBitmap );
			m_nCompBitmapWidth  = nTargetWidth + 20;
			m_nCompBitmapHeight = nTargetHeight + 4;
		}
		::SetWindowOrgEx( hdc, lpdis->rcItem.left, lpdis->rcItem.top, nullptr );
	}else{
		hdc = lpdis->hDC;
	}

	// 作画範囲を背景色で矩形塗りつぶし
	if( lpdis->itemState & ODS_SELECTED ){
		// アイテムが選択されている
		RECT rc1 = lpdis->rcItem;
		if( bMenuIconDraw
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
#else
			&& -1 != m_menuItems[nItemIndex].m_nBitmapIdx || lpdis->itemState & ODS_CHECKED
#endif
		){
			//rc1.left += (nIndentLeft - 3);
		}
#ifdef DRAW_MENU_SELECTION_LIGHT
		HPEN hPenBorder = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_HIGHLIGHT ) );
		HPEN hOldPen = (HPEN)::SelectObject( hdc, hPenBorder );
		COLORREF colHilight = ::GetSysColor( COLOR_HIGHLIGHT );
		COLORREF colMenu = ::GetSysColor( COLOR_MENU );
		BYTE valR = ((GetRValue(colHilight) * 4 + GetRValue(colMenu) * 6) / 10) | 0x18;
		BYTE valG = ((GetGValue(colHilight) * 4 + GetGValue(colMenu) * 6) / 10) | 0x18;
		BYTE valB = ((GetBValue(colHilight) * 4 + GetBValue(colMenu) * 6) / 10) | 0x18;
		HBRUSH hBrush = ::CreateSolidBrush( RGB(valR, valG, valB) );
		HBRUSH hOldBrush = (HBRUSH)::SelectObject( hdc, hBrush );
		::Rectangle( hdc, rc1.left, rc1.top, rc1.right, rc1.bottom );
		::SelectObject( hdc, hOldPen );
		::SelectObject( hdc, hOldBrush );
		::DeleteObject( hPenBorder );
		::DeleteObject( hBrush );
#else
		/* 選択ハイライト矩形 */
		::MyFillRect( hdc, rc1, COLOR_HIGHLIGHT );
#endif
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
	}else if( bMenuIconDraw ){
		// アイコン部分の背景を灰色にする
		CMyRect rcFillMenuBack( rcItem );
		rcFillMenuBack.left += nIndentLeft;
		::MyFillRect( hdc, rcFillMenuBack, COLOR_MENU );

//		hBrush = ::GetSysColorBrush( COLOR_3DFACE );
		COLORREF colMenu   = ::GetSysColor( COLOR_MENU );
		COLORREF colFace = ::GetSysColor( COLOR_3DFACE );
		COLORREF colIconBack;
		// 明度らしきもの
		if( 64 < t_abs(t_max(t_max(GetRValue(colFace),GetGValue(colFace)),GetBValue(colFace))
			         - t_max(t_max(GetRValue(colMenu),GetGValue(colMenu)),GetBValue(colMenu))) ){
			colIconBack = colFace;
		}else{
			// 明るさが近いなら混色にして(XPテーマ等で)違和感を減らす
			BYTE valR = ((GetRValue(colFace) * 7 + GetRValue(colMenu) * 3) / 10);
			BYTE valG = ((GetGValue(colFace) * 7 + GetGValue(colMenu) * 3) / 10);
			BYTE valB = ((GetBValue(colFace) * 7 + GetBValue(colMenu) * 3) / 10);
			colIconBack = RGB(valR, valG, valB);
		}
		
		CMyRect rcIconBk( rcItem );
		rcIconBk.right = rcItem.left + nIndentLeft;
		::MyFillRect( hdc, rcIconBk, colIconBack );

	}else{
		// アイテム矩形塗りつぶし
		::MyFillRect( hdc, lpdis->rcItem, COLOR_MENU );
	}
#else
	}else{
		::MyFillRect( hdc, lpdis->rcItem, COLOR_MENU );
	}
#endif

	if( bMenuIconDraw ){
		// アイコンとテキストの間に縦線を描画する
		int nSepColor = (::GetSysColor(COLOR_3DSHADOW) != ::GetSysColor(COLOR_MENU) ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
		HPEN hPen = ::CreatePen( PS_SOLID, cxBorder, ::GetSysColor(nSepColor) );
		HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, lpdis->rcItem.left + nIndentLeft, lpdis->rcItem.top, nullptr );
		::LineTo(   hdc, lpdis->rcItem.left + nIndentLeft, lpdis->rcItem.bottom );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );

	}
	
	if( lpdis->itemID == F_0 ){
		// セパレータの作画(セパレータのFuncCodeはF_SEPARETORではなくF_0)
		int y = lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top) / 2;
		int nSepColor = (::GetSysColor(COLOR_3DSHADOW) != ::GetSysColor(COLOR_MENU) ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
		HPEN hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor(nSepColor) );
		HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, lpdis->rcItem.left + (bMenuIconDraw ? nIndentLeft : cxEdge + cxBorder) + cxEdge, y, nullptr );
		::LineTo(   hdc, lpdis->rcItem.right - cxEdge, y );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		
		if( bBackSurface ){
			::BitBlt( hdcOrg, lpdis->rcItem.left, lpdis->rcItem.top, nTargetWidth, nTargetHeight,
				hdc, lpdis->rcItem.left, lpdis->rcItem.top, SRCCOPY );
		}
		return; // セパレータ。作画終了
	}

	// テキスト前景色を決定する
	COLORREF textColor;
	if( lpdis->itemState & ODS_DISABLED ){
		// アイテムが使用不可(淡色表示にする)
		textColor = ::GetSysColor( COLOR_GRAYTEXT );
	}else if( lpdis->itemState & ODS_SELECTED ){
#ifdef DRAW_MENU_SELECTION_LIGHT
		textColor = ::GetSysColor( COLOR_MENUTEXT );
#else
		textColor = ::GetSysColor( COLOR_HIGHLIGHTTEXT );
#endif
	}else{
		textColor = ::GetSysColor( COLOR_MENUTEXT );
	}

#ifdef _DEBUG
	// デバッグ用：メニュー項目に対して、ヘルプがない場合に前景色を青くする
	// メニュー項目に関する情報を取得します。
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
	mii.fMask = MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
	if( 0 != ::GetMenuItemInfo( (HMENU)lpdis->hwndItem, lpdis->itemID, FALSE, &mii )
	 && nullptr == mii.hSubMenu
	 && 0 == ::FuncID_To_HelpContextID( (EFunctionCode)lpdis->itemID ) 	/* 機能IDに対応するメニューコンテキスト番号を返す */
	){
		//@@@ 2001.12.21 YAZAKI
		if( lpdis->itemState & ODS_SELECTED ){
			textColor = ::GetSysColor( COLOR_HIGHLIGHTTEXT );	//	ハイライトカラー
		}
		else {
			textColor = RGB( 0, 0, 255 );	//	青くしてる。
		}
	}
#endif

	// テキスト矩形(インデント込み)
	CMyRect rcText( lpdis->rcItem );
	rcText.left += nIndentLeft + cxSmIcon / 4;
	rcText.right -= nIndentRight;

	const int nItemIndex = Find( (int)lpdis->itemID );
	LPCWSTR pszItemStr = m_menuItems[nItemIndex].m_cmemLabel.GetStringPtr();
	size_t nItemStrLen = m_menuItems[nItemIndex].m_cmemLabel.GetStringLength();

	int nBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
	HFONT hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );
	COLORREF textColorOld = (COLORREF)::SetTextColor( hdc, textColor );

	/* TAB文字の前と後ろに分割してテキストを描画する */
	size_t j;
	for( j = 0; j < nItemStrLen; ++j ){
		if( pszItemStr[j] == L'\t' ){
			break;
		}
	}
	/* TAB文字の前側のテキストを描画する */
	::DrawText(
		hdc,
		pszItemStr,
		static_cast<int>(j),
		&rcText,
		DT_LEFT | DT_VCENTER | DT_SINGLELINE
	);
	/* TAB文字の後ろ側のテキストを描画する */
	if( j < nItemStrLen ){
		::DrawText(
			hdc,
			&pszItemStr[j + 1],
			static_cast<int>(nItemStrLen - ( j + 1 )),
			&rcText,
			DT_RIGHT | DT_VCENTER | DT_SINGLELINE
		);
	}
	::SetTextColor( hdc, textColorOld );
	::SelectObject( hdc, hFontOld  );
	::SetBkMode( hdc, nBkModeOld );

	// アイコン矩形
	CMyRect rcIcon( rcItem );
	rcIcon.left += ( rcItem.Height() - m_pcIcons->cy() ) / 2;
	rcIcon.top += ( rcItem.Height() - m_pcIcons->cy() ) / 2;
	rcIcon.SetSize( m_pcIcons->cx(), m_pcIcons->cy() );

	// 枠は アイコン横幅xメニュー縦幅で表示し真ん中にアイコンを置く
	if( bMenuIconDraw && (lpdis->itemState & ODS_CHECKED) ){
		{
			// フラットな枠 + 半透明の背景色
			CMyRect rcFrame( rcIcon );
			::InflateRect( &rcFrame, cxEdge * 2, cyEdge * 2 );
			::MyFillRect( hdc, rcFrame, COLOR_HIGHLIGHT );

			COLORREF colHilight = ::GetSysColor( COLOR_HIGHLIGHT );
			COLORREF colMenu = ::GetSysColor( COLOR_MENU );
			// 16bitカラーの黒色でも少し明るくするように or 0x18 する
			BYTE valR;
			BYTE valG;
			BYTE valB;
			if( lpdis->itemState & ODS_SELECTED ){	// 選択状態
				valR = ((GetRValue(colHilight) * 6 + GetRValue(colMenu) * 4) / 10) | 0x18;
				valG = ((GetGValue(colHilight) * 6 + GetGValue(colMenu) * 4) / 10) | 0x18;
				valB = ((GetBValue(colHilight) * 6 + GetBValue(colMenu) * 4) / 10) | 0x18;
			} else {								// 非選択状態
				valR = ((GetRValue(colHilight) * 2 + GetRValue(colMenu) * 8) / 10) | 0x18;
				valG = ((GetGValue(colHilight) * 2 + GetGValue(colMenu) * 8) / 10) | 0x18;
				valB = ((GetBValue(colHilight) * 2 + GetBValue(colMenu) * 8) / 10) | 0x18;
			}
			CMyRect rcBkFrame( rcIcon );
			::InflateRect( &rcBkFrame, cxEdge , cyEdge );
			::MyFillRect( hdc, rcBkFrame, RGB( valR, valG, valB ) );
		}
	}

	/* 機能の画像が存在するならメニューアイコン?を描画する */
	if( bMenuIconDraw && -1 != m_menuItems[nItemIndex].m_nBitmapIdx ){
		// アイコン番号
		int nIconNo = m_menuItems[nItemIndex].m_nBitmapIdx;

		// メニューアイコン描画
		m_pcIcons->DrawToolIcon(
			hdc,
			rcIcon.left,
			rcIcon.top,
			nIconNo,
			( lpdis->itemState & ODS_DISABLED ) ? ILD_MASK : ILD_NORMAL,
			cxSmIcon,
			cySmIcon
		);

	}else{
		// チェックボックスを表示
		if( lpdis->itemState & ODS_CHECKED ){
			/* チェックマークの表示 */
			if( bMenuIconDraw ){
				// だいたい中心座標
				int nX = rcItem.left + rcIcon.Height() / 2;
				int nY = rcIcon.top + rcIcon.Height() /2;
				HPEN hPen   = nullptr;
				HPEN hPenOld = nullptr;
				// 2010.05.31 チェックの色を黒(未指定)からテキスト色に変更
				hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor(COLOR_MENUTEXT) );
				hPenOld = (HPEN)::SelectObject( hdc, hPen );
#if 0
// チェックマークも自分で書く場合
				if( !bMenuIconDraw ){
					nX -= 4; // iconがない場合、左マージン=2アイコン枠=2分がない
				}
#endif
				const int nBASE = 100*100; // 座標,nScale共に0.01単位
				// 16dot幅しかないので 1.0倍から2.1倍までスケールする(10-23)
				const int nScale = t_max(100, t_min(210, int((lpdis->rcItem.bottom - lpdis->rcItem.top - 2) * 100) / (16-2) ));
				for( int nBold = 1; nBold <= (281*nScale)/nBASE; nBold++ ){
					::MoveToEx( hdc, nX - (187*nScale)/nBASE, nY - (187*nScale)/nBASE, nullptr );
					::LineTo(   hdc, nX -   (0*nScale)/nBASE, nY -   (0*nScale)/nBASE );
					::LineTo(   hdc, nX + (468*nScale)/nBASE, nY - (468*nScale)/nBASE );
					nY++;
				}
				if( hPen ){
					::SelectObject( hdc, hPenOld );
					::DeleteObject( hPen );
				}
			}else{
				// OSにアイコン作画をしてもらう(黒背景等対応)
				HDC hdcMem = ::CreateCompatibleDC( hdc );
				HBITMAP hBmpMono = ::CreateBitmap( nCxCheck, nCyCheck, 1, 1, nullptr );
				HBITMAP hOld = (HBITMAP)::SelectObject( hdcMem, hBmpMono );
				RECT rcCheck = {0,0, nCxCheck, nCyCheck};
				::DrawFrameControl( hdcMem, &rcCheck, DFC_MENU, DFCS_MENUCHECK );
				COLORREF colTextOld = ::SetTextColor(hdc, RGB(0,0,0) );
				COLORREF colBackOld = ::SetBkColor(hdc,   RGB(255,255,255) );
				::BitBlt( hdc, lpdis->rcItem.left+2, lpdis->rcItem.top+2, nCxCheck, nCyCheck, hdcMem, 0, 0, SRCAND );
				::SetTextColor( hdc, textColor );
				::SetBkColor( hdc, RGB(0,0,0) );
				::BitBlt( hdc, lpdis->rcItem.left+2, lpdis->rcItem.top+2, nCxCheck, nCyCheck, hdcMem, 0, 0, SRCPAINT );
				::SetTextColor( hdc, colTextOld );
				::SetBkColor( hdc, colBackOld );
				::SelectObject( hdcMem, hOld );
				::DeleteObject( hBmpMono );
				::DeleteDC( hdcMem );
			}
		}
	}
	if( bBackSurface ){
		::BitBlt( hdcOrg, lpdis->rcItem.left, lpdis->rcItem.top, nTargetWidth, nTargetHeight,
			hdc, lpdis->rcItem.left, lpdis->rcItem.top, SRCCOPY );
	}
	return;
}

/*!
	作画終了
	メニューループ終了時に呼び出すとリソース節約になる
	
	@date 20100724 Moca バックサーフェス用に新設
*/
void CMenuDrawer::EndDrawMenu()
{
	DeleteCompDC();
}

void CMenuDrawer::DeleteCompDC()
{
	if( m_hCompDC ){
		::SelectObject( m_hCompDC, m_hCompBitmapOld );
		::DeleteObject( m_hCompBitmap );
		::DeleteObject( m_hCompDC );
//		DEBUG_TRACE( L"CMenuDrawer::DeleteCompDC %x\n", m_hCompDC );
		m_hCompDC = nullptr;
		m_hCompBitmap = nullptr;
		m_hCompBitmapOld = nullptr;
	}
}

/*
	ツールバー登録のための番号を返す。
	プラグインのみボタンのindexのかわりにidCommandをそのまま返す
	@date 2010.06.24 Moca 新規作成
	@note この値がiniのツールバーアイテムの記録に使われる
*/
int CMenuDrawer::FindToolbarNoFromCommandId( int idCommand, bool bOnlyFunc ) const
{
	// 先に存在確認をする
	int index = FindIndexFromCommandId( idCommand, bOnlyFunc );
	if( -1 < index ){
		// 固定部分以外(プラグインなど)はindexではなくidCommandのままにする
		if( m_nMyButtonFixSize <= index ){
			// もし コマンド番号が明らかに小さいと区別がつかない
			assert_warning( idCommand < m_nMyButtonFixSize );
			return idCommand;
		}
	}
	return index;
}

/** コマンドコードからツールバーボタン情報のINDEXを得る

	@param idCommand [in] コマンドコード
	@param bOnlyFunc [in] 有効な機能の範囲で検索する

	@retval みつからなければ-1を返す。

	@date 2005.08.09 aroka m_nMyButtonNum隠蔽のため追加
	@date 2005.11.02 ryoji bOnlyFuncパラメータを追加
 */
int CMenuDrawer::FindIndexFromCommandId( int idCommand, bool bOnlyFunc ) const
{
	if( bOnlyFunc ){
		// 機能の範囲外（セパレータや折り返しなど特別なもの）は除外する
		if ( !( F_MENU_FIRST <= idCommand && idCommand < F_MENU_NOT_USED_FIRST )
			&& !( F_PLUGCOMMAND_FIRST <= idCommand && idCommand < F_PLUGCOMMAND_LAST )){
			return -1;
		}
	}

	int nIndex = -1;
	for( int i = 0; i < m_nMyButtonNum; i++ ){
		if( m_tbMyButton[i].idCommand == idCommand ){
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

/** インデックスからボタン情報を得る

	@param nToolbarNo [in] ボタン情報のツールバー番号
	@retval ボタン情報

	@date 2007.11.02 ryoji 範囲外の場合は未定義のボタン情報を返すように
	@date 2010.06.24 Moca 引数をツールバー番号に変更
 */
TBBUTTON CMenuDrawer::getButton( int nToolbarNo ) const
{
	int index = ToolbarNoToIndex( nToolbarNo );
	if( 0 <= index && index < m_nMyButtonNum ){
		return m_tbMyButton[index];
	}

	// 範囲外なら未定義のボタン情報を作成して返す
	// （sakura.iniに範囲外インデックスが指定があった場合など、堅牢性のため）
	static TBBUTTON tbb;
	SetTBBUTTONVal( &tbb, -1, F_DISABLE, 0, TBSTYLE_BUTTON, 0, 0 );
	return tbb;
}

int CMenuDrawer::Find( int nFuncID )
{
	int i;
	int nItemNum = (int)m_menuItems.size();
	for( i = 0; i < nItemNum; ++i ){
		if( nFuncID == m_menuItems[i].m_nFuncId ){
			break;
		}
	}
	if( i >= nItemNum ){
		return -1;
	}else{
		return i;
	}
}

const WCHAR* CMenuDrawer::GetLabel( int nFuncID )
{
	int i;
	if( -1 == ( i = Find( nFuncID ) ) ){
		return nullptr;
	}
	return m_menuItems[i].m_cmemLabel.GetStringPtr();
}

WCHAR CMenuDrawer::GetAccelCharFromLabel( const WCHAR* pszLabel )
{
	int i;
	int nLen = (int)wcslen( pszLabel );
	for( i = 0; i + 1 < nLen; ++i ){
		if( L'&' == pszLabel[i] ){
			if( L'&' == pszLabel[i + 1]  ){
				i++;
			}else{
				return (WCHAR)towupper( pszLabel[i + 1] );
			}
		}
	}
	return L'\0';
}

struct WorkData{
	int				idx;
	MENUITEMINFO	mii;
};

/*! メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
LRESULT CMenuDrawer::OnMenuChar( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WCHAR				chUser;
	HMENU				hmenu;
	int i;
	chUser = (WCHAR) LOWORD(wParam);	// character code
	hmenu = (HMENU) lParam;				// handle to menu
//	MYTRACE( L"::GetMenuItemCount( %xh )==%d\n", hmenu, ::GetMenuItemCount( hmenu ) );

	//	Oct. 27, 2000 genta
	if( 0 <= chUser && chUser < ' '){
		chUser += '@';
	}
	else {
		chUser = (WCHAR)towupper( chUser );
	}

	// 2011.11.18 vector化
	std::vector<WorkData> vecAccel;
	size_t nAccelSel = 99999;
	for( i = 0; i < ::GetMenuItemCount( hmenu ); i++ ){
		WCHAR	szText[1024];
		// メニュー項目に関する情報を取得します。
		MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
		mii.fType = MFT_STRING;
		wcscpy( szText, L"--unknown--" );
		mii.dwTypeData = szText;
		mii.cch = _countof( szText ) - 1;
		if( 0 == ::GetMenuItemInfo( hmenu, i, TRUE, &mii ) ){
			continue;
		}
		const WCHAR* pszLabel;
		if( nullptr == ( pszLabel = GetLabel( mii.wID ) ) ){
			continue;
		}
		if( chUser == GetAccelCharFromLabel( pszLabel ) ){
			WorkData work;
			work.idx = i;
			work.mii = mii;
			if( /*-1 == nAccelSel ||*/ MFS_HILITE & mii.fState ){
				nAccelSel = vecAccel.size();
			}
			vecAccel.push_back( work );
		}
	}
//	MYTRACE( L"%d\n", (int)mapAccel.size() );
	if( 0 == vecAccel.size() ){
		return  MAKELONG( 0, MNC_IGNORE );
	}
	if( 1 == vecAccel.size() ){
		return  MAKELONG( vecAccel[0].idx, MNC_EXECUTE );
	}
//	MYTRACE( L"nAccelSel=%d vecAccel.size()=%d\n", nAccelSel, vecAccel.size() );
	if( nAccelSel + 1 >= vecAccel.size() ){
//		MYTRACE( L"vecAccel[0].idx=%d\n", vecAccel[0].idx );
		return  MAKELONG( vecAccel[0].idx, MNC_SELECT );
	}else{
//		MYTRACE( L"vecAccel[nAccelSel + 1].idx=%d\n", vecAccel[nAccelSel + 1].idx );
		return  MAKELONG( vecAccel[nAccelSel + 1].idx, MNC_SELECT );
	}
}

//	Jul. 21, 2003 genta
//	コメントアウトされていた部分を削除 (CImageListで再利用)

/* TBBUTTON構造体にデータをセット */
void CMenuDrawer::SetTBBUTTONVal(
	TBBUTTON*	ptb,
	int			iBitmap,
	int			idCommand,
	BYTE		fsState,
	BYTE		fsStyle,
	DWORD_PTR	dwData,
	INT_PTR		iString
) const
{
	/*
typedef struct _TBBUTTON {
	int iBitmap;	// ボタン イメージの 0 から始まるインデックス
	int idCommand;	// ボタンが押されたときに送られるコマンド
	BYTE fsState;	// ボタンの状態--以下を参照
	BYTE fsStyle;	// ボタン スタイル--以下を参照
	DWORD dwData;	// アプリケーション-定義された値
	int iString;	// ボタンのラベル文字列の 0 から始まるインデックス
} TBBUTTON;
*/

	ptb->iBitmap	= iBitmap;
	ptb->idCommand	= idCommand;
	ptb->fsState	= fsState;
	ptb->fsStyle	= fsStyle;
	ptb->dwData		= dwData;
	ptb->iString	= iString;
	return;
}

//ツールバーボタンを追加する
//	マネージメント機能追加	2010/7/3 Uchi 
//		全ウィンドウで同じ機能番号の場合、同じICON番号を持つように調整
void CMenuDrawer::AddToolButton( int iBitmap, int iCommand )
{
	TBBUTTON tbb;
	int 	iCmdNo;
	int 	i;
	
	if (m_pShareData->m_maxTBNum < m_nMyButtonNum) {
		m_pShareData->m_maxTBNum = m_nMyButtonNum;
	}

	if (iCommand >= F_PLUGCOMMAND_FIRST && iCommand <= F_PLUGCOMMAND_LAST) {
		iCmdNo = iCommand - F_PLUGCOMMAND_FIRST;
		if (m_pShareData->m_PlugCmdIcon[iCmdNo] != 0) {
			if (m_tbMyButton.size() <= (size_t)(int)m_pShareData->m_PlugCmdIcon[iCmdNo]) {
				// このウィンドウで未登録
				// 空きを詰め込む
				SetTBBUTTONVal( &tbb,TOOLBAR_ICON_PLUGCOMMAND_DEFAULT-1, 0, 0, TBSTYLE_BUTTON, 0, 0 );
				for (i = m_tbMyButton.size(); i < m_pShareData->m_PlugCmdIcon[iCmdNo]; i++) {
					m_tbMyButton.push_back( tbb );
					m_nMyButtonNum++;
				}

				// 未登録
				SetTBBUTTONVal( &tbb, iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );
				//最後に追加に変更
				m_tbMyButton.push_back( tbb );
				m_nMyButtonNum++;
			}
			else {
				// 再設定
				SetTBBUTTONVal( &m_tbMyButton[m_pShareData->m_PlugCmdIcon[iCmdNo]],
					iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );
			}
		}
		else {
			// 全体で未登録
			if (m_tbMyButton.size() < (size_t)m_pShareData->m_maxTBNum) {
				// 空きを詰め込む
				SetTBBUTTONVal( &tbb, TOOLBAR_ICON_PLUGCOMMAND_DEFAULT-1, 0, 0, TBSTYLE_BUTTON, 0, 0 );
				for (i = m_tbMyButton.size(); i < m_pShareData->m_maxTBNum; i++) {
					m_tbMyButton.push_back( tbb );
					m_nMyButtonNum++;
				}
			}
			// 新規登録
			SetTBBUTTONVal( &tbb, iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );

			m_pShareData->m_PlugCmdIcon[iCmdNo] = (short)m_tbMyButton.size();
			//最後から２番目に挿入する。一番最後は番兵で固定。
			//2010.06.23 Moca 最後に追加に変更
			m_tbMyButton.push_back( tbb );
			m_nMyButtonNum++;
		}
	}
	if (m_pShareData->m_maxTBNum < m_nMyButtonNum) {
		m_pShareData->m_maxTBNum = m_nMyButtonNum;
	}
}
