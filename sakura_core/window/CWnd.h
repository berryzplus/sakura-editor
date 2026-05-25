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
	virtual void	OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags);
	virtual void	OnLButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags);
	virtual void	OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags);
	virtual void	OnLButtonDblClk(HWND hWnd, int x, int y, UINT keyFlags);
	virtual void	OnRButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags);
	virtual void	OnMButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags);

	HWND		m_hWnd = nullptr;		// このウィンドウのハンドル
};

/*!
 * @brief カスタムウィンドウの基底クラス
 *
 * @par カスタムウィンドウの基本的な機能
 * @li 作成済みウィンドウにアタッチ
 * @li DispatchEventで特定メッセージに対する振る舞いを差し替える
 * @li ウィンドウからデタッチ
 */
class CCustomizedWnd : public CWnd
{
private:
	using Base = CWnd;
	using Me = CCustomizedWnd;

public:
	static LRESULT CALLBACK SubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
	);

	// コンストラクタは流用する
	using Base::Base;

	virtual bool	Attach(HWND hWnd, UINT uIdSubclass = 1);
	virtual void	Detach(HWND hWnd);

	LRESULT DefWndProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const override;

	UINT		m_IdSubclass = 0;
};

/*!
 * @brief 独自ウィンドウの基本クラス
 * 
 * @par CWndクラスの基本的な機能
 * @li ウィンドウ作成
 * @li ウィンドウメッセージ配送
 * 
 * @par 独自ウィンドウの使用方法は以下の手順
 * @li RegisterClassW()	ウィンドウクラス登録
 * @li CreateWnd()		ウィンドウ作成
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
	ATOM RegisterClassW(
		HBRUSH		hbrBackground,
		HCURSOR		hCursor,
		UINT		style = CS_DBLCLKS,
		int			cbWndExtra = 0,
		HICON		hIcon = nullptr,
		LPCWSTR		lpszMenuName = nullptr,
		HICON		hIconSm = nullptr
	) const;

	//ウィンドウ作成
	HWND CreateWnd(
		ATOM				atom,			// Pointer to a null-terminated string or is an atom.
		DWORD				dwStyle,		// window style
		HWND				hWndParent,
		size_t				windowId,		// handle to menu, or child-window identifier
		const CMyRect&		rc,				// window rect
		const std::optional<std::wstring>& optCaption = std::nullopt,
		DWORD				dwExStyle = 0
	) const;

	HINSTANCE	GetAppInstance() const noexcept { return m_hInstance; }
	HWND		GetParentHwnd() const noexcept { return m_hwndParent; }

	/* 仮想関数 */
	LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual LRESULT DispatchEvent_WM_APP(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/* 仮想関数 メッセージ処理(デフォルト動作) */
	virtual bool	OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);

	/* デフォルトメッセージ処理 */
	virtual LRESULT CallDefWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const;

	LRESULT DefWndProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const override
	{
		return CallDefWndProc(hWnd, uMsg, wParam, lParam);
	}

	//ウィンドウ標準操作
	void	DestroyWindow() const;

	std::wstring	m_ClassName;
	HINSTANCE		m_hInstance = nullptr;			//!< アプリケーションインスタンスのハンドル。（ウィンドウ作成後のみ有効）
	HWND			m_hwndParent = nullptr;			//!< 親ウィンドウのハンドル
};

#endif /* SAKURA_CWND_86C8E4DA_7921_4D79_A481_E3AB0557D767_H_ */
