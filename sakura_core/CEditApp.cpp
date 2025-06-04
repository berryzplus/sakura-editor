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
#include "_main/CAppMode.h"
#include "util/module.h"
#include "util/shell.h"

void CEditApp::Create()
{
	//ドキュメントの作成
	m_pcEditDoc = std::make_unique<CEditDoc>();

	//IO管理
	auto pcVisualProgress = std::make_unique<CVisualProgress>();

	//編集モード
	auto pcAppMode = std::make_unique<CAppMode>();	//ウィンドウよりも前にイベントを受け取るためにここでインスタンス作成

	//ドキュメントの作成
	m_pcEditDoc->Create();

	//ウィンドウの作成
	m_pcEditWnd = std::make_unique<CEditWnd>(std::move(pcVisualProgress), std::move(pcAppMode));
}
