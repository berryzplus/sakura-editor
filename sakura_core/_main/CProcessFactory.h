/*!	@file
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

#include "_main/CControlProcess.h"
#include "_main/CNormalProcess.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス生成クラス

	与えられたコマンドライン引数から生成すべきプロセスの種別を判定し，
	対応するオブジェクトを返すFactoryクラス．
 */
class CProcessFactory {
public:
	HINSTANCE m_hInstance;

	explicit CProcessFactory(_In_opt_ HINSTANCE hInstance = nullptr) noexcept
		: m_hInstance(hInstance)
	{
	}

	std::unique_ptr<CProcess> CreateInstance(std::wstring_view cmdLine) noexcept;

private:
	bool    ProfileSelect() noexcept;
};

#endif /* SAKURA_CPROCESSFACTORY_5006562F_7795_40FF_AA4C_FFB94842F7C5_H_ */
