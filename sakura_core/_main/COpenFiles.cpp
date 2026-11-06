/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "_main/COpenFiles.hpp"

#include "_main/CControlTray.h"
#include "_main/CMutex.h"
#include "_main/CProcessFactory.h"
#include "basis/make_initialization_error.hpp"

#include <chrono>

#include "config/system_constants.h"

#include "CSelectLang.h"

using namespace cxx;

bool COpenFiles::InitializeProcess(
	int nCmdShow [[maybe_unused]]
)
{
	// エディタ初期化中ミューテックスを用意する（エディタの初期化は排他制御する）
	std::wstring strMutexInitName = GSTR_MUTEX_SAKURA_INIT;
	strMutexInitName += GetProfileName();
	CMutex hMutex{ strMutexInitName, nullptr, true };

	// エディタ初期化中ミューテックス
	try
	{
		// ミューテックスを作成する
		if (!hMutex.try_lock_for(std::chrono::seconds(15))) {
			throw basis::make_initialization_error(
				// L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"
				LS(STR_ERR_DLGPROCFACT5)
				, static_cast<int>(EAppExitCode::Exited)	//エラーなのに正常っぽいコードを返すのは妥当か？
			);
		}
	}
	// エラーが発生した場合
	catch (const std::system_error&)
	{
		throw basis::make_initialization_error(
			// L"CreateMutex()失敗。\n終了します。"
			LS(STR_ERR_DLGNRMPROC1)
			, static_cast<int>(EAppExitCode::Exited)	//エラーなのに正常っぽいコードを返すのは妥当か？
		);
	}

	// スコープを抜けるときハンドルを閉じる
	using HandleHolder = cxx::ResourceHolder<&::CloseHandle>;

	// エディター初期化完了イベントを開く
	SFilePath initEventName{ std::format(GSTR_EVENT_SAKURA_EP_INITIALIZED, ::GetCurrentThreadId()) };
	HandleHolder hEvent{ ::OpenEventW(STANDARD_RIGHTS_REQUIRED | EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, initEventName) };

	// スコープを抜けるときシグナル状態になるようにする
	using EventHolder = cxx::ResourceHolder<&::SetEvent>;
	EventHolder initEvent{ hEvent.get() };

	// コントロールプロセスが起動している状態にする
	if (!CProcessFactory::IsExistControlProcess() &&
		!CProcessFactory::StartControlProcess())
	{
		return false;
	}

	// 共有メモリを初期化する
	GetShareData().OpenShareData();

	/* 言語を選択する */
	CSelectLang::ChangeLang(GetDllShareData().m_Common.m_sWindow.m_szLanguageDll);

	hMutex.unlock();

	OpenFiles();

	return false;	// 常に失敗させる
}
