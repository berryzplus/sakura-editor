/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CAppMode.h"
#include "window/CEditWnd.h"
#include "env/CSakuraEnvironment.h"

//! デバッグモニタモードに設定
void CAppMode::SetDebugModeON()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_sHandles.m_hwndDebug ){
		if( IsSakuraMainWindow( pShare->m_sHandles.m_hwndDebug ) ){
			return;
		}
	}
	pShare->m_sHandles.m_hwndDebug = CEditWnd::getInstance()->GetHwnd();
	this->_SetDebugMode(true);
	this->SetViewMode(false);	// ビューモード	// 2001/06/23 N.Nakatani アウトプット窓への出力テキストの追加F_ADDTAIL_Wが抑止されるのでとりあえずビューモードは辞めました
	CEditWnd::getInstance()->UpdateCaption();
}

// 2005.06.24 Moca
//! デバックモニタモードの解除
void CAppMode::SetDebugModeOFF()
{
	DLLSHAREDATA* pShare = &GetDllShareData();
	if( pShare->m_sHandles.m_hwndDebug == CEditWnd::getInstance()->GetHwnd() ){
		pShare->m_sHandles.m_hwndDebug = nullptr;
		this->_SetDebugMode(false);
		CEditWnd::getInstance()->UpdateCaption();
	}
}
