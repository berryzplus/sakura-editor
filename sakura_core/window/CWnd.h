/*!	@file
	@brief ウィンドウの基本クラス

	@author Norio Nakatani
	@date 2000/01/11 新規作成
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_
#define SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_
#pragma once

#include <Windows.h>
#include <oleacc.h>

#include "cxx/TComImpl.hpp"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//!	ウィンドウの基本クラス
/*!
	@par CWndクラスの基本的な機能
	@li ウィンドウ作成
	@li ウィンドウメッセージ配送

	@par 普通?のウィンドウの使用方法は以下の手順
	@li RegisterWC()	ウィンドウクラス登録
	@li Create()		ウィンドウ作成
*/
class CWnd
{

	using Me = CWnd;

protected:
	friend LRESULT CALLBACK CWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
public:
	/* Constructors */
	CWnd(const WCHAR* pszInheritanceAppend = L"");
	CWnd(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CWnd(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CWnd();

	/*
	||  Attributes & Operations
	*/

	// ウィンドウクラス登録
	ATOM RegisterWC(
		HINSTANCE	hInstance,
		HICON		hIcon,			// Handle to the class icon.
		HICON		hIconSm,		// Handle to a small icon
		HCURSOR		hCursor,		// Handle to the class cursor.
		HBRUSH		hbrBackground,	// Handle to the class background brush.
		LPCWSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		LPCWSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
	);

	//ウィンドウ作成
	HWND Create(
		HWND		hwndParent,
		DWORD		dwExStyle,		// extended window style
		LPCWSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
		LPCWSTR		lpWindowName,	// pointer to window name
		DWORD		dwStyle,		// window style
		int			x,				// horizontal position of window
		int			y,				// vertical position of window
		int			nWidth,			// window width
		int			nHeight,		// window height
		HMENU		hMenu			// handle to menu, or child-window identifier
	);

	virtual LRESULT DispatchEvent( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );/* メッセージ配送 */
protected:
	/* 仮想関数 */
	virtual LRESULT DispatchEvent_WM_APP( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */

	/* 仮想関数 メッセージ処理(デフォルト動作) */
	#define DECLH(method) LRESULT method( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ){return CallDefWndProc( hwnd, msg, wp, lp );}
	virtual DECLH( OnCommand		);	// WM_COMMAND
	virtual DECLH( OnPaint			);	// WM_PAINT
	virtual DECLH( OnLButtonDown	);	// WM_LBUTTONDOWN
	virtual DECLH( OnLButtonUp		);	// WM_LBUTTONUP
	virtual DECLH( OnLButtonDblClk	);	// WM_LBUTTONDBLCLK
	virtual DECLH( OnRButtonDown	);	// WM_RBUTTONDOWN
	virtual DECLH( OnMButtonDown	);	// WM_MBUTTONDOWN
	virtual DECLH( OnMouseMove		);	// WM_MOUSEMOVE
	virtual DECLH( OnTimer			);	// WM_TIMER
	virtual DECLH( OnSize			);	// WM_SIZE
	virtual DECLH( OnDestroy		);	// WM_DSESTROY

	virtual DECLH( OnMeasureItem	);	// WM_MEASUREITEM
	virtual DECLH( OnNotify			);	// WM_NOTIFY	//@@@ 2003.05.31 MIK
	virtual DECLH( OnDrawItem		);	// WM_DRAWITEM	// 2006.02.01 ryoji
	virtual DECLH( OnCaptureChanged	);	// WM_CAPTURECHANGED	// 2006.11.30 ryoji

	/* デフォルトメッセージ処理 */
	virtual LRESULT CallDefWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );

public:
	//インターフェース
	HWND GetHwnd() const{ return m_hWnd; }
	HWND GetParentHwnd() const{ return m_hwndParent; }
	HINSTANCE GetAppInstance() const{ return m_hInstance; }

	//特殊インターフェース (使用は好ましくない)
	void _SetHwnd(HWND hwnd){ m_hWnd = hwnd; }

	//ウィンドウ標準操作
	void DestroyWindow();

private: // 2002/2/10 aroka アクセス権変更
	HINSTANCE	m_hInstance = nullptr;	// アプリケーションインスタンスのハンドル
	HWND		m_hwndParent = nullptr;	// オーナーウィンドウのハンドル
	HWND		m_hWnd = nullptr;		// このダイアログのハンドル
#ifdef _DEBUG
	WCHAR		m_szClassInheritances[1024];
#endif
};

class CAccessible : public cxx::TComImpl<IAccessible>
{
private:
	using Base = cxx::TComImpl<IAccessible>;
	using Me = CAccessible;

public:
	CAccessible() = default;
	~CAccessible() override = default;

	STDMETHODIMP GetTypeInfoCount(UINT*) override
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo**) override
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*) override
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP get_accParent(IDispatch** ppdispParent) override
	{
		if (!ppdispParent) return E_POINTER;

		*ppdispParent = nullptr;

		return S_FALSE;
	}

	STDMETHODIMP get_accChildCount(long* pcountChildren) override
	{
		if (!pcountChildren) return E_POINTER;

		*pcountChildren = 0;

		return S_OK;
	}

	STDMETHODIMP get_accChild(VARIANT varChild, IDispatch** ppdispChild) override
	{
		if (!ppdispChild) return E_POINTER;

		*ppdispChild = nullptr;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accName(VARIANT varChild, BSTR* pszName) override
	{
		if (!pszName) return E_POINTER;

		*pszName = nullptr;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accValue(VARIANT varChild, BSTR* pszValue) override
	{
		if (!pszValue) return E_POINTER;

		*pszValue = nullptr;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accDescription(VARIANT varChild, BSTR* pszDescription) override
	{
		if (!pszDescription) return E_POINTER;

		*pszDescription = nullptr;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accRole(VARIANT varChild, VARIANT* pvarRole) override
	{
		if (!pvarRole) return E_POINTER;

		::VariantInit(pvarRole);

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accState(VARIANT varChild, VARIANT* pvarState) override
	{
		if (!pvarState) return E_POINTER;

		::VariantInit(pvarState);

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accHelp(VARIANT varChild, BSTR* pszHelp) override
	{
		if (!pszHelp) return E_POINTER;

		*pszHelp = nullptr;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic) override
	{
		if (!pszHelpFile || !pidTopic) return E_POINTER;

		*pszHelpFile = nullptr;
		*pidTopic = 0;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut) override
	{
		if (!pszKeyboardShortcut) return E_POINTER;

		*pszKeyboardShortcut = nullptr;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP get_accFocus(VARIANT* pvarChild) override
	{
		if (!pvarChild) return E_POINTER;

		::VariantInit(pvarChild);

		return S_FALSE;
	}

	STDMETHODIMP get_accSelection(VARIANT* pvarChildren) override
	{
		if (!pvarChildren) return E_POINTER;

		::VariantInit(pvarChildren);

		return S_FALSE;
	}

	STDMETHODIMP get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction) override
	{
		if (!pszDefaultAction) return E_POINTER;

		*pszDefaultAction = nullptr;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP accSelect(long, VARIANT varChild) override
	{
		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild) override
	{
		if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight) return E_POINTER;

		*pxLeft = 0;
		*pyTop = 0;
		*pcxWidth = 0;
		*pcyHeight = 0;

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return S_FALSE;
	}

	STDMETHODIMP accNavigate(long, VARIANT varChild, VARIANT* pvarEndUpAt) override
	{
		if (!pvarEndUpAt) return E_POINTER;

		::VariantInit(pvarEndUpAt);

		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		return E_NOTIMPL;
	}

	STDMETHODIMP accHitTest(long, long, VARIANT* pvarChild) override
	{
		if (!pvarChild) return E_POINTER;

		::VariantInit(pvarChild);

		// このメソッドはオーバーラードされる前提。
		// この位置でヒットテストを実施する

		return S_FALSE;
	}

	STDMETHODIMP accDoDefaultAction(VARIANT varChild) override
	{
		if (VT_I4 != varChild.vt || CHILDID_SELF != varChild.lVal) {
			return E_INVALIDARG;
		}

		// このメソッドはオーバーラードされる前提。
		// この位置でデフォルトアクションを実行する

		return S_FALSE;
	}

	STDMETHODIMP put_accName(VARIANT, BSTR) override
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP put_accValue(VARIANT, BSTR) override
	{
		return E_NOTIMPL;
	}
};

#endif /* SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_ */
