﻿/*!	@file
	@brief プロセス生成クラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CPROCESSFACTORY_5006562F_7795_40FF_AA4C_FFB94842F7C5_H_
#define SAKURA_CPROCESSFACTORY_5006562F_7795_40FF_AA4C_FFB94842F7C5_H_
#pragma once

#include "_main/CCommandLine.h"
#include "_main/CProcess.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス生成クラス

	与えられたコマンドライン引数から生成すべきプロセスの種別を判定し，
	対応するオブジェクトを返すFactoryクラス．
 */
class CProcessFactory {
private:
	using CommandLineHolder = std::unique_ptr<CCommandLine>;
	using ProcessHolder = std::unique_ptr<CProcess>;

	HINSTANCE         m_hInstance;
	int               m_nCmdShow;
	CommandLineHolder m_pCommandLine = nullptr;

public:
	explicit CProcessFactory(
		_In_ HINSTANCE hInstance = GetModuleHandleW(nullptr),
		_In_ int nCmdShow = SW_SHOWDEFAULT
	) noexcept;

	ProcessHolder CreateInstance(std::wstring_view commandLine);

private:
	bool ProfileSelect(HINSTANCE hInstance) const;
};

#endif /* SAKURA_CPROCESSFACTORY_5006562F_7795_40FF_AA4C_FFB94842F7C5_H_ */
