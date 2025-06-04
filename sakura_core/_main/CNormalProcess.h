/*!	@file
	@brief エディタプロセスクラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#define SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_
#pragma once

#include "CProcess.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "util/design_template.h"

#include "extmodule/CMigemo.h"

class CDlgGrep;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief エディタプロセスクラス
	
	エディタプロセスはCEditWndクラスのインスタンスを作る。
*/
class CNormalProcess final : public CProcess {
private:
	using CEditDocHolder = std::unique_ptr<CEditDoc>;
	using CEditWndHolder = std::unique_ptr<CEditWnd>;

public:
	static bool StartControlProcess(_In_opt_z_ LPCWSTR pszProfileName);

	using CProcess::CProcess;
	~CNormalProcess() override = default;

protected:
	//プロセスハンドラ
	bool InitializeProcess() override;
	bool MainLoop() override;
	void OnExitProcess() override;

protected:
	//実装補助
	HANDLE _GetInitializeMutex() const; // 2002/2/8 aroka
	bool    OpenFiles(EditInfo& fi, const std::vector<std::wstring>& files) const;
	bool    ApplyGrepOptions(CDlgGrep& cDlgGrep) const noexcept;

private:
	CEditDocHolder		m_pcEditDoc = nullptr;	//!< ドキュメント
	CEditWndHolder		m_pcEditWnd = nullptr;	//!< 編集ウィンドウ

	CMigemo		m_cMigemo;
};

#endif /* SAKURA_CNORMALPROCESS_F2808B31_61DC_4BE0_8661_9626478AC7F9_H_ */
