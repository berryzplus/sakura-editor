/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CAUTOSCROLLWND_F588E196_7D77_4DFA_AAB0_A2D95FFB8849_H_
#define SAKURA_CAUTOSCROLLWND_F588E196_7D77_4DFA_AAB0_A2D95FFB8849_H_
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "window/CWnd.h"

class CEditView;
class CMyPoint;

class CAutoScrollWnd : public COriginalWnd
{
private:
	using BitmapHolder = cxx::ResourceHolder<&::DeleteObject, HBITMAP>;
	using MemDcHolder = cxx::ResourceHolder<&::DeleteDC>;
	using SelectionHolder = cxx::ResourceHolder<&::SelectObject>;

	using Base = COriginalWnd;
	using Me = CAutoScrollWnd;

public:
	static std::unique_ptr<CAutoScrollWnd> CreateInstance(bool bVertical, bool bHorizontal);

protected:
	CAutoScrollWnd(bool bVertical, bool bHorizontal);

public:
	~CAutoScrollWnd() override;

	HWND Create( HINSTANCE hInstance, HWND hwndParent, bool bVertical, bool bHorizontal,
				 const CMyPoint& point, CEditView* view );

protected:
	/* 仮想関数 */
	LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	bool	OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) override;
	void	OnDestroy(HWND hWnd) override;
	void	OnPaint(HWND hWnd, PAINTSTRUCT& ps) override;

	void	ExitAutoScroll(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags);

private:
	int				m_BitMapId;
	int				m_CursorId;
	BitmapHolder	m_hCenterImg = nullptr;
	CEditView*		m_cView = nullptr;
};

#endif /* SAKURA_CAUTOSCROLLWND_F588E196_7D77_4DFA_AAB0_A2D95FFB8849_H_ */
