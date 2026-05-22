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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_
#define SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_
#pragma once

#include <Windows.h>

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

struct SLresult
{
	LRESULT result = 0L;
	bool handled = false;

	SLresult() = default;

	explicit SLresult(LRESULT result)
		: result(result)
		, handled(true)
	{
	}

	explicit operator bool() const noexcept
	{
		return handled;
	}
};

/*!
 * @brief ウィンドウの基底クラス
 */
class CWnd
{
private:
	using Me = CWnd;

public:
	/* Constructors */
	CWnd();
	CWnd(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CWnd(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CWnd() = default;

	/*
	||  Attributes & Operations
	*/
	virtual LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT DefWndProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return ::SendMessageW(hWnd, uMsg, wParam, lParam);
	}

	//インターフェース
	HWND		GetHwnd() const noexcept { return m_hWnd; }

	//特殊インターフェース (使用は好ましくない)
	void		_SetHwnd(HWND hwnd) { m_hWnd = hwnd; }

	virtual void	OnDestroy(HWND hWnd);
	virtual void	OnSize(HWND hWnd, UINT state, int cx, int cy);
	virtual void	OnPaint(HWND hWnd, PAINTSTRUCT& ps);
	virtual void	OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT* lpDrawItem);
	virtual void	OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* lpMeasureItem);

	virtual LRESULT	OnNotify(HWND hWnd, UINT_PTR idFrom, LPNMHDR pNMHDR);

	virtual void	OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT notifyCode);
	virtual void	OnTimer(HWND hWnd, UINT id);

	HWND		m_hWnd = nullptr;		// このウィンドウのハンドル
};

/*!
 * @brief 独自ウィンドウの基本クラス
 * 
 * @par CWndクラスの基本的な機能
 * @li ウィンドウ作成
 * @li ウィンドウメッセージ配送
 * 
 * @par 独自ウィンドウの使用方法は以下の手順
 * @li RegisterWC()	ウィンドウクラス登録
 * @li Create()		ウィンドウ作成
 */
class COriginalWnd : public CWnd
{
private:
	using Base = CWnd;
	using Me = COriginalWnd;

public:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	explicit COriginalWnd(std::wstring_view className)
		: m_ClassName(className)
	{
	}

	~COriginalWnd() override;

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

	HINSTANCE	GetAppInstance() const noexcept { return m_hInstance; }
	HWND		GetParentHwnd() const noexcept { return m_hwndParent; }

	/* 仮想関数 */
	LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual LRESULT DispatchEvent_WM_APP(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/* 仮想関数 メッセージ処理(デフォルト動作) */

#pragma push_macro("DECLH")

#define DECLH(method) LRESULT method(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWndProcW(hWnd, uMsg, wParam, lParam); }

	virtual DECLH( OnLButtonDown	);	// WM_LBUTTONDOWN
	virtual DECLH( OnLButtonUp		);	// WM_LBUTTONUP
	virtual DECLH( OnLButtonDblClk	);	// WM_LBUTTONDBLCLK
	virtual DECLH( OnRButtonDown	);	// WM_RBUTTONDOWN
	virtual DECLH( OnMButtonDown	);	// WM_MBUTTONDOWN
	virtual DECLH( OnMouseMove		);	// WM_MOUSEMOVE

	virtual DECLH( OnCaptureChanged	);	// WM_CAPTURECHANGED	// 2006.11.30 ryoji

#pragma pop_macro("DECLH")

	/* デフォルトメッセージ処理 */
	virtual LRESULT CallDefWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

	LRESULT DefWndProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const override
	{
		return CallDefWndProc(hWnd, uMsg, wParam, lParam);
	}

	//ウィンドウ標準操作
	void	DestroyWindow() const;

	std::wstring	m_ClassName;
	HINSTANCE		m_hInstance = G_AppInstance();	//!< アプリケーションインスタンスのハンドル
	HWND			m_hwndParent = nullptr;			//!< 親ウィンドウのハンドル
};

#endif /* SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_ */
