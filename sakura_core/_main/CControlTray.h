/*!	@file
	@brief 常駐部

	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
	@date 2007/10/23 kobake     クラス名、ファイル名変更: CEditApp→CControlTray
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, Stonee, aroka, genta
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CCONTROLTRAY_E9E24D69_3511_4EC1_A29A_1D119F68004A_H_
#define SAKURA_CCONTROLTRAY_E9E24D69_3511_4EC1_A29A_1D119F68004A_H_
#pragma once

#include "dlg/CDlgGrep.h"
#include "env/DLLSHAREDATA.h"
#include "env/CPropertyManager.h"
#include "uiparts/CImageListMgr.h"
#include "uiparts/CMenuDrawer.h"

struct SLoadInfo;
struct EditInfo;

//!	常駐部の管理
/*!
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CControlTray
{
private:
	using CPropertyManagerHolder = std::unique_ptr<CPropertyManager>;

	/*!
	 * トレイアイコン再登録要求のメッセージID。
	 *
	 * Windows エクスプローラーが再起動したときに送出される。
	 *
	 * 独自メッセージは、システムグローバルな領域に登録される。
	 * 同じ名前に対しては、同じメッセージIDが返される仕様なので
	 * init only のグローバル定数とみなすことができる。
	 *
	 * @date 2001/04/24 genta
	 */
	static inline const UINT gm_uMsgTaskbarCreated = ::RegisterWindowMessageW(L"TaskbarCreated");

public:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	/*
	||  Constructors
	*/
	CControlTray();
	~CControlTray();

	/*
	|| メンバ関数
	*/
	HWND Create(HINSTANCE hInstance);	/* 作成 */

	LRESULT DispatchEvent(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);	/* メッセージ処理 */
	void MessageLoop( void );	/* メッセージループ */
	int	CreatePopUpMenu_L( void );	/* ポップアップメニュー(トレイ左ボタン) */
	int	CreatePopUpMenu_R( void );	/* ポップアップメニュー(トレイ右ボタン) */

	//ウィンドウ管理
	static bool OpenNewEditor(							//!< 新規編集ウィンドウの追加 ver 0
		HINSTANCE			hInstance,					//!< [in] インスタンスID (実は未使用)
		HWND				hWndParent,					//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
		const SLoadInfo&	sLoadInfo,					//!< [in]
		const WCHAR*		szCmdLineOption	= nullptr,		//!< [in] 追加のコマンドラインオプション
		bool				sync			= false,	//!< [in] trueなら新規エディタの起動まで待機する
		const WCHAR*		pszCurDir		= nullptr,		//!< [in] 新規エディタのカレントディレクトリ
		bool				bNewWindow		= false		//!< [in] 新規エディタをウインドウで開く
	);
	static bool OpenNewEditor2(						//!< 新規編集ウィンドウの追加 ver 1
		HINSTANCE		hInstance,
		HWND			hWndParent,
		const EditInfo*	pfi,
		bool			bViewMode,
		bool			sync		= false,
		bool			bNewWindow	= false
	);
	static void ActiveNextWindow(HWND hwndParent);
	static void ActivePrevWindow(HWND hwndParent);

	static BOOL CloseAllEditor( BOOL bCheckConfirm, HWND hWndFrom, BOOL bExit, int nGroup );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更	// 2006.12.25, 2007.02.13 ryoji 引数追加
	static void TerminateApplication( HWND hWndFrom );	/* サクラエディタの全終了 */	// 2006.12.25 ryoji 引数追加

public:
	HWND GetTrayHwnd() const{ return m_hWnd; }

	/*
	|| 実装ヘルパ系
	*/
	static void DoGrepCreateWindow(HINSTANCE hinst, HWND, CDlgGrep& cDlgGrep);
protected:
	void	DoGrep();	//Stonee, 2001/03/21
	void	RegisterHotKey(HWND hWnd) noexcept;

	void OnNewEditor(bool bNewWindow); //!< 2003.05.30 genta 新規ウィンドウ作成処理を切り出し

private:
	bool	OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	void	OnDestroy(HWND hWnd);
	void	OnClose(HWND hWnd) const;
	bool	OnQueryEndSession(HWND hWnd, UINT endSessionFlags) const;
	void	OnEndSession(HWND hWnd, bool bEndSession, UINT endSessionFlags);
	void	OnHelp(HWND hWnd, const HELPINFO* lpHelpInfo) const noexcept;
	void	OnTimer(HWND hWnd, UINT id);
	void	OnHotKey(HWND hWnd, int idHotKey, UINT fuModifiers, UINT vk) const;

	bool	OnSetTypeSetting(size_t index);
	bool	OnGetTypeSetting(size_t index);
	bool	OnAddTypeSetting(size_t index);
	bool	OnDelTypeSetting(size_t index);

	/*
	|| メンバ変数
	*/
	HINSTANCE		m_hInstance = nullptr;
	HWND			m_hWnd = nullptr;

	DLLSHAREDATA*	m_pShareData = GetDllShareDataPtr();
	SFilePath		m_szLanguageDll;

	CImageListMgr	m_hIcons;
	CMenuDrawer		m_cMenuDrawer;

	BOOL			m_bCreatedTrayIcon = FALSE;		//!< トレイにアイコンを作った

	CPropertyManagerHolder	m_pcPropertyManager = std::make_unique<CPropertyManager>();

	// DispatchEventから切り出した変数群（そのうちリネームする）
	HWND			hwndHtmlHelp = nullptr;
	WORD			wHotKeyMods = 0;
	WORD			wHotKeyCode = 0;
	bool			bLDClick = false;		//<! 左ダブルクリックをしたか

	bool			m_bUseTrayMenu = false;	//<! トレイメニュー表示中

	CDlgGrep		m_cDlgGrep;
	int				m_nCurSearchKeySequence = -1;
};

#endif /* SAKURA_CCONTROLTRAY_E9E24D69_3511_4EC1_A29A_1D119F68004A_H_ */
