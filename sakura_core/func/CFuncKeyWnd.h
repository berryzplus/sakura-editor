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
#include "window/CWnd.h"
#include "env/DLLSHAREDATA.h"

struct DLLSHAREDATA;
class CEditDoc; // 2002/2/10 aroka

//! ファンクションキーウィンドウ
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CFuncKeyWnd final : public COriginalWnd
{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;

	using Base = COriginalWnd;
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
	HWND Open( HINSTANCE, HWND, CEditDoc*, bool );	/* ウィンドウ オープン */
	void Close( void );	/* ウィンドウ クローズ */
	void SizeBox_ONOFF(bool bSizeBox);	/* サイズボックスの表示／非表示切り替え */
	void Timer_ONOFF(bool bStart); /* 更新の開始／停止 20060126 aroka */
	/*
	|| メンバ変数
	*/
private:
	// 20060126 aroka すべてPrivateにして、初期化順序に合わせて並べ替え
	CEditDoc*		m_pcEditDoc = nullptr;
	DLLSHAREDATA*	m_pShareData;
	int				m_nCurrentKeyState = -1;
	WCHAR			m_szFuncNameArr[12][256];
	HWND			m_hwndButtonArr[12];
	FontHolder		m_hFont = nullptr;	/*!< 表示用フォント */
	bool			m_bSizeBox = false;
	HWND			m_hwndSizeBox = nullptr;
	int				m_nTimerCount = 0;
	int				m_nButtonGroupNum; // Openで初期化
	EFunctionCode	m_nFuncCodeArr[12]; // Open->CreateButtonsで初期化
protected:
	/*
	|| 実装ヘルパ系
	*/
	void CreateButtons( void );	/* ボタンの生成 */
	int		CalcButtonWidth(int cx);

	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	void	OnDestroy(HWND hWnd) override;
	void	OnSize(HWND hWnd, UINT state, int cx, int cy) override;
	void	OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT notifyCode) override;
	LRESULT OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;	// WM_TIMERタイマーの処理
};

#endif /* SAKURA_CFUNCKEYWND_2EB0FD88_ABBB_4280_BEEA_46E8468E4550_H_ */
