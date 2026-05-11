/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CEditApp.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "_main/CCommandLine.h"
#include "util/module.h"
#include "util/shell.h"

void CEditApp::Create(HINSTANCE hInst, int nGroupId)
{
	m_hInst = hInst;

	//ヘルパ作成
	m_cIcons.Create( m_hInst );	//	CreateImage List

	//ドキュメントの作成
	m_pcEditDoc->Create();

	//ウィンドウの作成
	m_pcEditWnd->Create( m_pcEditDoc, &m_cIcons, nGroupId );

	//プロパティ管理
	m_pcPropertyManager->Create(
		m_pcEditWnd->GetHwnd(),
		&GetIcons(),
		&m_pcEditWnd->GetMenuDrawer()
	);
}

CEditApp::~CEditApp()
{
	delete m_pcEditWnd;
	delete m_pcEditDoc;
}

/*! 共通設定 プロパティシート */
bool CEditApp::OpenPropertySheet( int nPageNum )
{
	/* プロパティシートの作成 */
	bool bRet = m_pcPropertyManager->OpenPropertySheet( m_pcEditWnd->GetHwnd(), nPageNum, false );
	if( bRet ){
		// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		m_pcSMacroMgr->UnloadAll();
	}

	return bRet;
}

/*! タイプ別設定 プロパティシート */
bool CEditApp::OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType )
{
	bool bRet = m_pcPropertyManager->OpenPropertySheetTypes( m_pcEditWnd->GetHwnd(), nPageNum, nSettingType );

	return bRet;
}
