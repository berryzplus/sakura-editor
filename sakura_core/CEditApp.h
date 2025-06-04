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
#include "types/CType.h"

class CEditWnd;
enum EFunctionCode;

//!エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
class CEditApp final : public TSingleton<CEditApp>{
private:
	friend class TSingleton<CEditApp>;
	CEditApp() = default;
	~CEditApp() = default;

	using CEditDocHolder = std::unique_ptr<CEditDoc>;
	using CEditWndHolder = std::unique_ptr<CEditWnd>;

public:
	void Create();

private:
	//ドキュメント
	CEditDocHolder		m_pcEditDoc = nullptr;

	//ウィンドウ
	CEditWndHolder		m_pcEditWnd = nullptr;
};

//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw(){ return "CAppExitException"; }
};

#endif /* SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_ */
