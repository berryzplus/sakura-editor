/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CEditApp.h"

#include "_main/CCommandLine.h"
#include "util/module.h"
#include "util/shell.h"

CEditApp::CEditApp()
{
	//ヘルパ作成
	m_cIcons.Create( m_hInst );	//	CreateImage List
}

void CEditApp::Create(HINSTANCE hInst [[maybe_unused]], int nGroupId)
{
	//ドキュメントの作成
	m_pcEditDoc->Create();

	//ウィンドウの作成
	m_pcEditWnd->Create( GetDocument(), &GetIcons(), nGroupId );

	//プロパティ管理
	m_pcPropertyManager->Create(
		m_pcEditWnd->GetHwnd(),
		&GetIcons(),
		&m_pcEditWnd->GetMenuDrawer()
	);
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
