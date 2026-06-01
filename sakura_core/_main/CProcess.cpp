/*!	@file
	@brief プロセス基底クラス

	@author aroka
	@date 2002/01/07 作成
	@date 2002/01/17 修正
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2004, Moca
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcess.h"

#include "config/app_constants.h"
#include "config/system_constants.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "util/module.h"

#include "CSelectLang.h"

/*!
 * @brief サクラエディタのプロセスを起動する
 *
 * @param si スタートアップ情報
 * @param args コマンドライン引数
 * @param optWorkingDir カレントディレクトリ（省略した場合は起動元と同じ）
 * @param optProfileName プロファイル名（省略した場合は指定なし）
 * @return 起動したプロセスのハンドルオブジェクト
 */
/* static */ cxx::ProcessHolder CProcess::CreateSakuraProcess(
	STARTUPINFO& si,
	std::vector<std::wstring>& args,
	const std::optional<std::filesystem::path>& optWorkingDir,
	const std::optional<std::wstring>& optProfileName
)
{
	// プロファイル指定がある場合、argsに追加する
	if (const auto pCommandLine = CCommandLine::getInstance(); (pCommandLine && pCommandLine->IsSetProfile()) || optProfileName.has_value()) {
		const auto profileName = optProfileName.value_or(GetProfileName());
		args.emplace(args.begin(), std::format(LR"(-PROF={})", CCommandLine::QuoteArg(profileName)));
	}

	// 実行ファイルのパスを取得する
	const auto exePath = GetExeFileName();

	// 引数を空白区切りで連結する
	auto strCommandLine = std::accumulate(args.begin(), args.end(), CCommandLine::QuoteArg(exePath.native()), [](const std::wstring& a, std::wstring_view b) { return std::format(LR"({} {})", a, CCommandLine::QuoteArg(b)); });

	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;

	LPCWSTR lpszWorkingDir = nullptr;
	if (optWorkingDir.has_value()) {
		if (const auto attr = ::GetFileAttributesW(optWorkingDir->c_str());
			INVALID_FILE_ATTRIBUTES != attr && (attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			lpszWorkingDir = optWorkingDir->c_str();
		}
	}

	// プロセス情報（出力用構造体なので値は入れない）
	PROCESS_INFORMATION pi{};

	// コントロールプロセスを起動する
	if (!::CreateProcessW(
		exePath.c_str(),		// 実行可能モジュールパス
		strCommandLine.data(),	// コマンドラインバッファ
		nullptr,				// プロセスのセキュリティ記述子
		nullptr,				// スレッドのセキュリティ記述子
		FALSE,					// ハンドルの継承オプション(継承させない)
		dwCreationFlag,			// 作成のフラグ
		nullptr,				// 環境変数(変更しない)
		lpszWorkingDir,			// カレントディレクトリ(変更しない)
		&si,					// スタートアップ情報
		&pi						// プロセス情報(作成されたプロセス情報を格納する構造体)
	))
	{
		cxx::raise_system_error("create process failed.");
	}

	// 開いたハンドルは使わないので閉じておく
	::CloseHandle(pi.hThread);

	return cxx::ProcessHolder{ pi.hProcess, pi.dwProcessId, pi.dwThreadId };
}

/*!
 * @brief コントロールプロセスを起動する
 *
 * @param optProfileName プロファイル名
 * @return 起動したプロセスオブジェクト
 */
/* static */ cxx::ProcessHolder CProcess::CreateControlProcess(
	const std::optional<std::wstring>& optProfileName
)
{
	std::wstring profileName{ optProfileName.value_or(GetProfileName()) };

	// トレイウインドウのクラス名を組み立てる
	std::wstring trayWndClassName{ GSTR_CEDITAPP };
	trayWndClassName += profileName;

	// トレイウインドウを検索する
	if (const auto hTrayWnd = cxx::FindWindowW(trayWndClassName, trayWndClassName)) {
		// トレイウインドウからプロセスIDを取得する
		DWORD dwControlProcessId;
		if (DWORD dwControlThreadId = ::GetWindowThreadProcessId(hTrayWnd, &dwControlProcessId)) {
			if (const auto process = ::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwControlProcessId)) {
				return cxx::ProcessHolder{ process, dwControlProcessId, dwControlThreadId };
			}
		}
	}

	// 初期化完了イベントの名前を決める
	SFilePath initEventName{ GSTR_EVENT_SAKURA_CP_INITIALIZED };
	initEventName += profileName;

	// プロセス起動前に初期化完了イベントを作成する
	cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, initEventName);
	if (!hEvent || ERROR_ALREADY_EXISTS == ::GetLastError()) {
		cxx::raise_system_error("create event failed.");
	}

	std::vector<std::wstring> commandArgs{ LR"(-NOWIN)" };

	std::wstring title{ L"sakura control process" };

	// スタートアップ情報（入力用構造体なので値を入れる）
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.lpTitle = std::data(title);
	si.wShowWindow = SW_SHOWDEFAULT;

	// コントロールプロセスを起動する
	auto cp = CreateSakuraProcess(si, commandArgs, cxx::GetSystemDirectoryW(), optProfileName);
	if (!cp) return cxx::ProcessHolder{};

	// 初期化完了を待つ
	std::array handles{ hEvent.get(), cp.get() };
	if (const auto dwRet = ::WaitForMultipleObjects(DWORD(std::size(handles)), std::data(handles), FALSE, 15000); WAIT_OBJECT_0 != dwRet) {
		cxx::raise_system_error("waitEvent is timeout.");
	}

	return cxx::ProcessHolder{ cp.release(), cp.dwProcessId, cp.dwThreadId };
}

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
*/
CProcess::CProcess(
	HINSTANCE				hInstance,		//!< handle to process instance
	CCommandLineHolder&&	pCommandLine	//!< pointer to command line
)
	: m_hInstance(hInstance)
	, m_pCommandLine(std::move(pCommandLine))
{
}

/*!
	@brief iniファイルパスを取得する
 */
std::filesystem::path CProcess::GetIniFileName() const
{
	if (m_cShareData.IsPrivateSettings()) {
		const DLLSHAREDATA *pShareData = &GetDllShareData();
		return pShareData->m_szPrivateIniFile.c_str();
	}
	return GetExeFileName().replace_extension(L".ini");
}

/*!
 * @brief プロセス実行
 *
 * @param nCmdShow [in] コマンドライン引数の表示オプション
 * @return PostQuitMessage()で指定した終了コード
 * 
 * @author aroka
 * @date 2002/01/16 新規作成
 */
int CProcess::Run(int nCmdShow)
{
	if (InitializeProcess(nCmdShow)) {
		if (const auto pcMainWnd = GetMainWnd()) {
			return pcMainWnd->MessageLoop();
		}
	}

	return 0;
}

/*!
	言語選択後に共有メモリ内の文字列を更新する
*/
void CProcess::RefreshString()
{
	m_cShareData.RefreshString();
}
