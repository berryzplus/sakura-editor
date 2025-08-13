/*! @file */
/*
	Copyright (C) 2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#include "basis/message_error.hpp"

#ifdef _MSC_VER

// MSVCにはMS Detoursを使う。
#include <detours/detours.h>

#else // MSVC以外

// MSVC以外はMinHookを使う。
#include <MinHook.h>

#endif  // MSVC以外

/*!
 * APIフック機構の極薄ラッパー
 */
namespace hooking {

#ifdef _MSC_VER

// MS DetoursはAPIラップ操作トランザクション制御する
template <typename F, typename T>
void do_transaction(F action, T targetFunc, T stubFunc)
{
	// トランザクションを開始する
	if (NO_ERROR != DetourTransactionBegin()) {
		throw basis::message_error(L"begin detour transaction failed.");
	}

	// トランザクションのスレッド状態を更新する
	if (NO_ERROR != DetourUpdateThread(GetCurrentThread())) {
		DetourTransactionAbort();
		throw basis::message_error(L"update detour transaction failed.");
	}

	// Detoursのアクションを実行する
	if (NO_ERROR != action(std::bit_cast<PVOID*>(&targetFunc), std::bit_cast<LPVOID>(stubFunc)))
	{
		DetourTransactionAbort();
		throw basis::message_error(L"detour transaction failed.");
	}

	// トランザクションをコミットする
	if (NO_ERROR != DetourTransactionCommit())
	{
		DetourTransactionAbort();
		throw basis::message_error(L"commit detour transaction failed.");
	}
}

#endif  // MSVC以外

template <typename T>
void attach(T targetFunc, T stubFunc)
{
#ifdef _MSC_VER

	do_transaction(&DetourAttach, targetFunc, stubFunc);

#else // MSVC以外（MinGW64とか）はMinHookを使う。

	// MinHookライブラリを初期化する
    if (MH_OK != MH_Initialize()) {
		throw basis::message_error(L"hook init failed.");
	}
	
	T pfnFunc = nullptr;

	// APIフックを作成する
	if (MH_OK != MH_CreateHook((LPVOID)targetFunc, (LPVOID)stubFunc, (LPVOID*)&pfnFunc)) {
		throw basis::message_error(L"hook creation failed.");
	}

	// APIフックを有効にする
	if (MH_OK != MH_EnableHook((LPVOID)targetFunc)) {
		throw basis::message_error(L"hook enabling failed.");
	}
#endif  // MSVC以外
}

template <typename T>
void detach(T targetFunc, T stubFunc)
{
#ifdef _MSC_VER

	do_transaction(&DetourDetach, targetFunc, stubFunc);

#else // MSVC以外（MinGW64とか）はMinHookを使う。

	// APIフックを無効にする
	if (MH_OK != MH_DisableHook((LPVOID)targetFunc)) {
		throw basis::message_error(L"hook detachment failed.");
	}

	// APIフックを削除する
	if (MH_OK != MH_RemoveHook((LPVOID)targetFunc)) {
		throw basis::message_error(L"hook removal failed.");
	}

	// MinHookライブラリを解放する
    if (MH_OK != MH_Uninitialize()) {
		throw basis::message_error(L"hook uninit failed.");
	}
#endif  // MSVC以外
}

} // namespace hooking

#define HOOK_ATTACH hooking::attach
#define HOOK_DETACH hooking::detach
