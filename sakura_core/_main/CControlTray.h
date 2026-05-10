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

#include "config/system_constants.h"
#include "cxx/TComImpl.hpp"
#include "dlg/CDlgGrep.h"
#include "env/DLLSHAREDATA.h"
#include "env/CPropertyManager.h"
#include "uiparts/CImageListMgr.h"
#include "uiparts/CMenuDrawer.h"

#include "sakura_h.h"

#ifdef __MINGW32__
#include "sakura_iid_decl.hpp"
#endif

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
	explicit CControlTray(ITrayWnd& refTrayWnd);
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

	HWND	GetHwnd() const noexcept { return m_hWnd; }
	HWND	GetTrayHwnd() const noexcept { return GetHwnd(); }

	/*
	|| 実装ヘルパ系
	*/
	static void DoGrepCreateWindow(HINSTANCE hinst, HWND, CDlgGrep& cDlgGrep);
protected:
	void	CreateTrayIcon();
	void	DoGrep();	//Stonee, 2001/03/21
	void	RegisterHotKey(HWND hWnd) noexcept;
	bool	SendTrayMessage(DWORD dwMessage, HICON hIcon = nullptr, const std::optional<std::wstring>& optTip = std::nullopt) const;

	void OnNewEditor(bool bNewWindow); //!< 2003.05.30 genta 新規ウィンドウ作成処理を切り出し

private:
	bool	OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);
	void	OnDestroy(HWND hWnd);
	void	OnClose(HWND hWnd) const noexcept;
	bool	OnQueryEndSession(HWND hWnd, UINT endSessionFlags) const noexcept;
	void	OnEndSession(HWND hWnd, bool bEndSession, UINT endSessionFlags) noexcept;
	LRESULT OnGetObject(HWND hWnd, WPARAM wParam, LONG dwObjId) const;
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

	cxx::com_pointer<ITrayWnd>		m_pTrayWnd = nullptr;

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

namespace cxx {

/*!
 * @brief COMクラスファクトリーテンプレート
 *
 * @tparam TImplType 生成するクラス
 *
 * @note 仮置き。いつか移動する。
 */
template<typename TImplType>
class TClassFactoryImpl final : public TComImpl<IClassFactory>
{
private:
	cxx::com_pointer<ITypeInfo> m_pTypeInfo;

	using Base = TComImpl<IClassFactory>;
	using Me   = TClassFactoryImpl<TImplType>;

public:
	static com_pointer_type make_instance(
		ITypeLib& refTypeLib
	)
	{
		cxx::com_pointer<ITypeInfo> pTypeInfo;
		_com_util::CheckError(refTypeLib.GetTypeInfoOfGuid(__uuidof(typename TImplType::com_type), &pTypeInfo));

		return Base::to_com_pointer(std::make_unique<Me>(*pTypeInfo));
	}

	explicit TClassFactoryImpl(ITypeInfo& refTypeInfo)
		: m_pTypeInfo(&refTypeInfo)
	{
	}

#pragma region ClassFactory
	IFACEMETHODIMP CreateInstance(
		_In_opt_ LPUNKNOWN  pUnkOuter,
		_In_ REFIID		 riid,
		_COM_Outptr_ void** ppvObject) override
	{
		if (!ppvObject)
		{
			return E_POINTER;
		}

		*ppvObject = nullptr;

		if (pUnkOuter)
		{
			return CLASS_E_NOAGGREGATION;
		}

		if (!(riid == __uuidof(typename TImplType::com_type) || riid == IID_IDispatch || riid == IID_IUnknown))
		{
			return E_NOINTERFACE;
		}

		auto pObject = TImplType::to_com_pointer(std::make_unique<TImplType>(*m_pTypeInfo, *this));

		*ppvObject = pObject.Detach();

		return S_OK;
	}

	IFACEMETHODIMP LockServer(
		BOOL fLock) override
	{
		if (fLock)
		{
			AddRef();
		}
		else
		{
			Release();
		}

		return S_OK;
	}

#pragma endregion
};

/*!
 * @brief COMクラス実装テンプレート
 *
 * @tparam TImplType 実装クラス
 * @tparam TargetInterface 実装するインターフェース
 *
 * @note 仮置き。いつか移動する。
 */
template<typename TImplType, typename TargetInterface>
class TClassImpl : public TComImpl<TargetInterface> {
private:
	ITypeInfo&		m_TypeInfo;
	IClassFactory&	m_ClassFactory;

	using Base = TComImpl<TargetInterface>;
	using Me = TClassImpl<TImplType, TargetInterface>;

public:
	using factory_type = cxx::TClassFactoryImpl<TImplType>;

	TClassImpl(
		ITypeInfo& TypeInfo_,
		IClassFactory& ClassFactory_
	)
		: m_TypeInfo(TypeInfo_)
		, m_ClassFactory(ClassFactory_)
	{
	}

#pragma region Unknown
	/*!
	 * 参照カウンターをインクリメントします。
	 *
	 * @returns 参照カウント
	 */
	IFACEMETHODIMP_(ULONG) AddRef() override
	{
		// 参照カウントをインクリメントする
		const auto nRefCount = Base::AddRef();

		// クラスファクトリをロックする
		m_ClassFactory.LockServer(TRUE);

		return nRefCount;
	}

	/*!
	 * 参照カウンターをデクリメントし、不要になったオブジェクトを破棄します。
	 *
	 * @returns 参照カウント
	 */
	IFACEMETHODIMP_(ULONG) Release() override
	{
		// クラスファクトリをアンロックする
		m_ClassFactory.LockServer(FALSE);

		// 参照カウントをデクリメントする
		return Base::Release();
	}

#pragma endregion
#pragma region Dispatch
	IFACEMETHODIMP GetTypeInfoCount(
		UINT*		pctinfo
	) override
	{
		if (!pctinfo)
		{
			return E_POINTER;
		}

		*pctinfo = 1;

		return S_OK;
	}

	IFACEMETHODIMP GetTypeInfo(
		UINT		iTInfo,
		LCID		lcid [[maybe_unused]],
		ITypeInfo**	ppTInfo
	) override
	{
		if (!ppTInfo)
		{
			return E_POINTER;
		}

		*ppTInfo = nullptr;

		if (iTInfo != 0)
		{
			return DISP_E_BADINDEX;
		}

		return m_TypeInfo.QueryInterface(IID_PPV_ARGS(ppTInfo));
	}

	IFACEMETHODIMP GetIDsOfNames(
		REFIID		riid [[maybe_unused]],
		LPOLESTR*	rgszNames,
		UINT		cNames,
		LCID		lcid [[maybe_unused]],
		DISPID*		rgDispId
	) override
	{
		return ::DispGetIDsOfNames(&m_TypeInfo, rgszNames, cNames, rgDispId);
	}

	IFACEMETHODIMP Invoke(
		DISPID		dispIdMember,
		REFIID		riid [[maybe_unused]],
		LCID		lcid [[maybe_unused]],
		WORD		wFlags,
		DISPPARAMS*	pDispParams,
		VARIANT*	pVarResult,
		EXCEPINFO*	pExcepInfo,
		UINT*		puArgErr
	) override
	{
		// このオブジェクトのメソッドに転送する
		return ::DispInvoke(this, &m_TypeInfo, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	}

#pragma endregion
};

} // namespace cxx

class CTrayWnd final : public cxx::TClassImpl<CTrayWnd, ITrayWnd> {
private:
	using Base = TClassImpl<CTrayWnd, ITrayWnd>;
	using Me = CTrayWnd;

public:
	using Base::Base;

	HWND GetHwnd() const
	{
		return GetDllShareData().m_sHandles.m_hwndTray;
	}

	IFACEMETHODIMP ShowTrayClickMenu() override
	{
		if (!::SendMessageTimeoutW(GetHwnd(), MYWM_NOTIFYICON, 0, WM_LBUTTONUP, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 0, nullptr)) return S_FALSE;

		return S_OK;
	}

	IFACEMETHODIMP ShowTrayContextMenu() override
	{
		if (!::SendMessageTimeoutW(GetHwnd(), MYWM_NOTIFYICON, 0, WM_RBUTTONUP, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 0, nullptr)) return S_FALSE;

		return S_OK;
	}

	IFACEMETHODIMP OpenNewEditor() override
	{
		if (!::SendMessageTimeoutW(GetHwnd(), MYWM_NOTIFYICON, 0, WM_LBUTTONDBLCLK, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 0, nullptr)) return S_FALSE;

		return S_OK;
	}
};

namespace cxx {

IClassFactory* CreateControlClassFactory();

} // namespace cxx

#endif /* SAKURA_CCONTROLTRAY_E9E24D69_3511_4EC1_A29A_1D119F68004A_H_ */
