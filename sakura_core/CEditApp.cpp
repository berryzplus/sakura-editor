/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CEditApp.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "uiparts/CVisualProgress.h"
#include "macro/CSMacroMgr.h"
#include "CPropertyManager.h"
#include "CGrepAgent.h"
#include "_main/CAppMode.h"
#include "_main/CCommandLine.h"
#include "util/module.h"
#include "util/shell.h"

void CEditApp::Create(HINSTANCE hInst)
{
	m_hInst = hInst;

	//ドキュメントの作成
	m_pcEditDoc = std::make_unique<CEditDoc>();

	//IO管理
	m_pcVisualProgress = new CVisualProgress();

	//GREPモード管理
	m_pcGrepAgent = new CGrepAgent();

	//編集モード
	CAppMode::getInstance();	//ウィンドウよりも前にイベントを受け取るためにここでインスタンス作成

	//マクロ
	m_pcSMacroMgr = new CSMacroMgr();

	//ドキュメントの作成
	m_pcEditDoc->Create();

	//ウィンドウの作成
	m_pcEditWnd = CEditWnd::getInstance();
}

CEditApp::~CEditApp()
{
	delete m_pcSMacroMgr;
	delete m_pcGrepAgent;
	delete m_pcVisualProgress;
}

bool CEditApp::IsGrepRunning() const noexcept
{
	return m_pcGrepAgent && m_pcGrepAgent->m_bGrepRunning;
}

/*! 共通設定 プロパティシート */
bool CEditWnd::OpenPropertySheet( int nPageNum )
{
	/* プロパティシートの作成 */
	bool bRet = m_pcPropertyManager->OpenPropertySheet(GetHwnd(), nPageNum, false);
	if( bRet ){
		// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		CEditApp::getInstance()->m_pcSMacroMgr->UnloadAll();
	}

	return bRet;
}

/*! タイプ別設定 プロパティシート */
bool CEditWnd::OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType )
{
	bool bRet = m_pcPropertyManager->OpenPropertySheetTypes(GetHwnd(), nPageNum, nSettingType);

	return bRet;
}
