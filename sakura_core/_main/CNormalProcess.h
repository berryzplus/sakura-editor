/*!	@file
	@brief エディタプロセスクラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#define SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#pragma once

#include "_main/global.h"
#include "_main/CProcess.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "extmodule/CMigemo.h"
#include "CEditApp.h"
#include "util/design_template.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief エディタプロセスクラス
	
	エディタプロセスはCEditWndクラスのインスタンスを作る。
*/
class CNormalProcess final : public CProcess {
private:
	using CEditAppHolder = std::unique_ptr<CEditApp>;
	using CEditDocHolder = std::unique_ptr<CEditDoc>;
	using CEditWndHolder = std::unique_ptr<CEditWnd>;

	using Base = CProcess;
	using Me = CNormalProcess;

public:
	//コンストラクタ・デストラクタ
	using Base::Base;
	~CNormalProcess() override;

private:
	//プロセスハンドラ
	bool	InitializeProcess(int nCmdShow) override;

	CAppMainWnd* GetMainWnd() const override {
		return m_pcEditWnd.get();
	}

	//実装補助
	HANDLE _GetInitializeMutex() const; // 2002/2/8 aroka
	void OpenFiles(HWND hwnd);

private:
	CEditDocHolder		m_pcEditDoc = nullptr;

	CEditWndHolder		m_pcEditWnd = nullptr;

	CEditAppHolder		m_pcEditApp = nullptr;

	CMigemo		m_cMigemo;
};

#endif /* SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_ */
