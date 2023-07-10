/*!	@file
	@brief 文字列共通定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, Stonee, jepro
	Copyright (C) 2002, KK
	Copyright (C) 2003, MIK
	Copyright (C) 2005, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/global.h"

#include "apimodule/User32Dll.hpp"
#include "apimodule/Kernel32Dll.hpp"
#include "apimodule/Shell32Dll.hpp"
#include "apimodule/Ole32Dll.hpp"
#include "apimodule/ComCtl32Dll.hpp"

#include <mutex>

#include "_main/CNormalProcess.h"
#include "basis/CErrorInfo.h"
#include "config/app_constants.h"
#include "window/CEditWnd.h"
#include "version.h"

#ifdef DEV_VERSION
#pragma message("-------------------------------------------------------------------------------------")
#pragma message("---  This is a Dev version and under development. Be careful to use this version. ---")
#pragma message("-------------------------------------------------------------------------------------")
#endif

/*!
	アプリ名を取得します。
	プロセスの生成前にアプリ名を取得することはできません。

	@date 2007/09/21 kobake 整理
 */
LPCWSTR GetAppName( void )
{
	const auto pcProcess = CProcess::getInstance();
	if( !pcProcess )
	{
		::_com_raise_error(E_FAIL, MakeMsgError(L"Any process has been instantiated."));
	}
	return pcProcess->GetAppName();
}

/*! 選択領域描画用パラメータ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;

HINSTANCE G_AppInstance()
{
	return CProcess::getInstance()->GetProcessInstance();
}

/*!
 * コンストラクタ
 */
SSearchOption::SSearchOption() noexcept
	: SSearchOption(false, false, false)
{
}

/*!
 * コンストラクタ(値指定)
 */
SSearchOption::SSearchOption(
	bool _bRegularExp,
	bool _bLoHiCase,
	bool _bWordOnly
) noexcept
	: bRegularExp(_bRegularExp)
	, bLoHiCase(_bLoHiCase)
	, bWordOnly(_bWordOnly)
{
}

//! リセットする(全部falseにする)
void SSearchOption::Reset()
{
	bRegularExp = false;
	bLoHiCase = false;
	bWordOnly = false;
}

/*!
 * 同型との等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しい
 * @retval false 等しくない
 */
bool SSearchOption::operator == (const SSearchOption& rhs) const noexcept
{
	if (this == &rhs) return true;
	return bRegularExp == rhs.bRegularExp
		&& bLoHiCase == rhs.bLoHiCase
		&& bWordOnly == rhs.bWordOnly;
}

/*!
 * 同型との否定の等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しくない
 * @retval false 等しい
 */
bool SSearchOption::operator != (const SSearchOption& rhs) const noexcept
{
	return !(*this == rhs);
}

const User32Dll& GetUser32Dll() {
	static std::unique_ptr<User32Dll> User32Dll_;
	if (!User32Dll_) {
		User32Dll_ = std::make_unique<User32Dll>();
	}
	return *User32Dll_;
}

const Kernel32Dll& GetKernel32Dll() {
	static std::unique_ptr<Kernel32Dll> Kernel32Dll_;
	if (!Kernel32Dll_) {
		Kernel32Dll_ = std::make_unique<Kernel32Dll>();
	}
	return *Kernel32Dll_;
}

const Shell32Dll& GetShell32Dll() {
	static std::unique_ptr<Shell32Dll> Shell32Dll_;
	if (!Shell32Dll_) {
		Shell32Dll_ = std::make_unique<Shell32Dll>();
	}
	return *Shell32Dll_;
}

const Ole32Dll& GetOle32Dll() {
	static std::unique_ptr<Ole32Dll> Ole32Dll_;
	if (!Ole32Dll_) {
		Ole32Dll_ = std::make_unique<Ole32Dll>();
	}
	return *Ole32Dll_;
}

const ComCtl32Dll& GetComCtl32Dll() {
	static std::unique_ptr<ComCtl32Dll> ComCtl32Dll_;
	if (!ComCtl32Dll_) {
		ComCtl32Dll_ = std::make_unique<ComCtl32Dll>();
	}
	return *ComCtl32Dll_;
}
