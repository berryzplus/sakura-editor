/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#define SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#pragma once

//2007.10.23 kobake 作成

#include "doc/CEditDoc.h"
#include "types/CType.h"
#include "util/design_template.h"
#include "window/CEditWnd.h"

//!エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
class CEditApp : public TSingleton<CEditApp>{
	friend class TSingleton<CEditApp>;
	CEditApp(){}
	virtual ~CEditApp();

public:
	void Create(HINSTANCE hInst, int);

	//モジュール情報
	HINSTANCE GetAppInstance() const{ return m_hInst; }	//!< インスタンスハンドル取得

	//ウィンドウ情報
	CEditWnd* GetEditWindow(){ return m_pcEditWnd; }		//!< ウィンドウ取得

	CEditDoc*		GetDocument(){ return m_pcEditDoc; }
	CImageListMgr&	GetIcons(){ return m_cIcons; }

	bool OpenPropertySheet( int nPageNum );
	bool OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType );

public:
	HINSTANCE			m_hInst;

	//ドキュメント
	CEditDoc*			m_pcEditDoc = new CEditDoc(nullptr);

	//ウィンドウ
	CEditWnd*			m_pcEditWnd = new CEditWnd();

	//IO管理
	CVisualProgress*	m_pcVisualProgress = m_pcEditDoc->m_pcVisualProgress.get();

	//その他ヘルパ
	CSMacroMgr*			m_pcSMacroMgr = m_pcEditDoc->m_pcSMacroMgr.get();			//マクロ管理

	CPropertyManager*	m_pcPropertyManager = m_pcEditWnd->m_pcPropertyManager.get();

	CGrepAgent*			m_pcGrepAgent = m_pcEditDoc->m_pcGrepAgent.get();			//GREPモード

	CSoundSet&			m_cSoundSet = m_pcEditWnd->m_cSoundSet;

	//GUIオブジェクト
	CImageListMgr&		m_cIcons = m_pcEditWnd->m_hIcons;
};

//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw() override{ return "CAppExitException"; }
};

#endif /* SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_ */
