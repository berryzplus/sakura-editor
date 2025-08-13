/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "pch.h"
#include "testing/MessageBoxHook.hpp"

#include "hooking/HookingLibrary.hpp"

namespace testing {

/*!
 * MessageBoxExWのスタブ関数
 *
 * 呼出引数をスタティック配列に記録し、あらかじめ設定した戻り値を返す。
 *
 * 本物のAPIを呼ばないのでUI表示待ち待機は発生しない。
 */
int WINAPI MessageBoxHook::MessageBoxExWStub(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId)
{
	std::lock_guard lock(gm_mtx);

	UNREFERENCED_PARAMETER(wLanguageId);

	if (gm_returnValues.empty()) {
		throw basis::message_error(L"Unexpected call of MessageBoxW");
	}

	gm_logs.emplace_back(
		hWnd,
		lpText ? lpText : L""s,
		lpCaption ? lpCaption : L""s,
		uType
	);

	const auto returnValue = gm_returnValues.back();
	gm_returnValues.pop_back();

	return returnValue;
}

/*!
 * コンストラクタ
 */
MessageBoxHook::MessageBoxHook(int returnValue)
{
	std::lock_guard lock(gm_mtx);

	push(returnValue);

	HOOK_ATTACH(&MessageBoxExW, &MessageBoxExWStub);
}

/*!
 * デストラクタ
 */
MessageBoxHook::~MessageBoxHook() noexcept
{
	std::lock_guard lock(gm_mtx);

	try {
		HOOK_DETACH(&MessageBoxExW, &MessageBoxExWStub);
	}
	catch (const basis::message_error&) {
		// 何もしない
	}

	gm_logs.clear();
	gm_returnValues.clear();
}

void MessageBoxHook::push(int returnValue)
{
	std::lock_guard lock(gm_mtx);

	gm_returnValues.push_back(returnValue);
	gm_logs.clear();
}

} // namespace testing
