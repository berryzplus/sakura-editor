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
	void Close();

protected:
	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	void	OnDestroy(HWND hWnd) override;
	void	OnPaint(HWND hWnd, PAINTSTRUCT& ps) override;
	void	OnLButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags) override;
	void	OnRButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags) override;
	void	OnMButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags) override;

private:
	int				m_BitMapId;
	int				m_CursorId;
	BitmapHolder	m_hCenterImg = nullptr;
	CEditView*		m_cView = nullptr;
};

struct CAutoScrollCWnd final : public CAutoScrollWnd
{
	CAutoScrollCWnd() : CAutoScrollWnd(true, true) {}
};

struct CAutoScrollVWnd final : public CAutoScrollWnd
{
	CAutoScrollVWnd() : CAutoScrollWnd(true, false) {}
};

struct CAutoScrollHWnd final : public CAutoScrollWnd
{
	CAutoScrollHWnd() : CAutoScrollWnd(false, false) {}
};

#endif /* SAKURA_CAUTOSCROLLWND_F588E196_7D77_4DFA_AAB0_A2D95FFB8849_H_ */
