/*!	@file
	@brief ファンクションキーウィンドウ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CFUNCKEYWND_2EB0FD88_ABBB_4280_BEEA_46E8468E4550_H_
#define SAKURA_CFUNCKEYWND_2EB0FD88_ABBB_4280_BEEA_46E8468E4550_H_
#pragma once

#include "_main/global.h"
#include "doc/CEditDoc.h"
#include "env/DLLSHAREDATA.h"
#include "window/CWnd.h"

//! ファンクションキーウィンドウ
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CFuncKeyWnd final : public TSizeBoxParent<COriginalWnd>
{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;

	using Base = TSizeBoxParent<COriginalWnd>;
	using Me = CFuncKeyWnd;

public:
	/*
	||  Constructors
	*/
	CFuncKeyWnd();
	~CFuncKeyWnd() override;

	/*
	|| メンバ関数
	*/
	HWND	Open(HWND hWndParent, const CMyRect& rc, bool bSizeBox);

	void Timer_ONOFF(bool bStart); /* 更新の開始／停止 20060126 aroka */

	/*
	|| メンバ変数
	*/
private:
	// 20060126 aroka すべてPrivateにして、初期化順序に合わせて並べ替え
	DLLSHAREDATA*	m_pShareData = &GetDllShareData();
	CEditDoc*		m_pcEditDoc = GetDocument();
	int				m_nCurrentKeyState = -1;
	WCHAR			m_szFuncNameArr[12][256];
	HWND			m_hwndButtonArr[12];
	FontHolder		m_hFont = nullptr;	/*!< 表示用フォント */
	int				m_nTimerCount = 0;
	int				m_nButtonGroupNum; // Openで初期化
	EFunctionCode	m_nFuncCodeArr[12]; // Open->CreateButtonsで初期化

	/*
	|| 実装ヘルパ系
	*/
	void	CreateButtons(HWND hWnd, HINSTANCE hInstance, int cx, int cy);
	int		CalcButtonWidth(int cx) const;

	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	bool	OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) override;
	void	OnDestroy(HWND hWnd) override;
	void	OnSize(HWND hWnd, UINT state, int cx, int cy) override;
	void	OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT notifyCode) override;
	void	OnTimer(HWND hWnd, UINT id) override;
};

#endif /* SAKURA_CFUNCKEYWND_2EB0FD88_ABBB_4280_BEEA_46E8468E4550_H_ */
