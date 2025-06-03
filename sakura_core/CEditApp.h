/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#define SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#pragma once

//2007.10.23 kobake 作成

#include "doc/CEditDoc.h"

#include "util/design_template.h"
#include "uiparts/CSoundSet.h"
#include "types/CType.h"

class CEditWnd;
class CVisualProgress;
class CGrepAgent;
enum EFunctionCode;

//!エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
class CEditApp final : public TSingleton<CEditApp>{
private:
	friend class TSingleton<CEditApp>;
	CEditApp() = default;
	~CEditApp();

	using CEditDocHolder = std::unique_ptr<CEditDoc>;

public:
	void Create(HINSTANCE hInst);

	//モジュール情報
	HINSTANCE GetAppInstance() const{ return m_hInst; }	//!< インスタンスハンドル取得

	//ウィンドウ情報
	CEditWnd* GetEditWindow(){ return m_pcEditWnd; }		//!< ウィンドウ取得

	CEditDoc*		GetDocument(){ return m_pcEditDoc.get(); }

public:
	HINSTANCE			m_hInst;

	//ドキュメント
	CEditDocHolder		m_pcEditDoc;

	//ウィンドウ
	CEditWnd*			m_pcEditWnd;

	//IO管理
	CVisualProgress*	m_pcVisualProgress;

	//その他ヘルパ
	CGrepAgent*			m_pcGrepAgent;			//GREPモード
	CSoundSet			m_cSoundSet;			//サウンド管理
};

//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw(){ return "CAppExitException"; }
};

#endif /* SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_ */
