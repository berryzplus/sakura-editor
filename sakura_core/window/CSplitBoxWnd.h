/*!	@file
	@brief 分割ボックスウィンドウクラス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CSPLITBOXWND_D85ABC4D_AF8F_4B42_B1E5_BA066925314E_H_
#define SAKURA_CSPLITBOXWND_D85ABC4D_AF8F_4B42_B1E5_BA066925314E_H_
#pragma once

#include "CWnd.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 分割ボックスウィンドウクラス
*/
class CSplitBoxWnd : public COriginalWnd
{
private:
	using Base = COriginalWnd;
	using Me = CSplitBoxWnd;

	/*
	||  Constructors
	*/
protected:
	explicit CSplitBoxWnd(bool bVertical);

public:
	~CSplitBoxWnd() override;

	HWND Create(HINSTANCE hInstance, HWND hwndParent, int bVertical);

	static void Draw3dRect(HDC hdc, int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight);
	static void FillSolidRect(HDC hdc, int x, int y, int cx, int cy, COLORREF clr);

//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージディスパッチャ */

protected:
	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	void	OnPaint(HWND hWnd, PAINTSTRUCT& ps) override;
	void	OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags) override;
	void	OnLButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags) override;
	void	OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags) override;
	void	OnLButtonDblClk(HWND hWnd, int x, int y, UINT keyFlags) override;

private:
	int			m_bVertical;	/* 垂直分割ボックスか */
	LPCWSTR		m_CursorName;
	int			m_nDragPosY = 0;
	int			m_nDragPosX = 0;
};

struct CVSplitBoxWnd final : public CSplitBoxWnd
{
	CVSplitBoxWnd() : CSplitBoxWnd(true) {}
};

struct CHSplitBoxWnd final : public CSplitBoxWnd
{
	CHSplitBoxWnd() : CSplitBoxWnd(false) {}
};

#endif /* SAKURA_CSPLITBOXWND_D85ABC4D_AF8F_4B42_B1E5_BA066925314E_H_ */
