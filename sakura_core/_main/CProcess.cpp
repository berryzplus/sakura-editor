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

#include "_main/CCommandLine.h"

#include "util/module.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "config/app_constants.h"
#include "CSelectLang.h"
#include "String_define.h"

//! HANDLE型のスマートポインタ
using HandleHolder = cxx_util::ResourceHolder<HANDLE, &CloseHandle>;

namespace cxx_util {

/*!
 * ワイド文字列をマルチバイト文字列に変換する
 */
std::string wcstombs_s(std::wstring_view wcs) {

	using LocaleHolder = ResourceHolder<_locale_t, &_free_locale>;

	// 現在のスレッドロケールを取得
	const LocaleHolder locale = _get_current_locale();
	if (!locale) {
		throw basis::message_error(L"Failed to get current locale");
	}

	// 変換に必要なバッファサイズを求める
	size_t required = 0;
	if (const auto ret = _wcstombs_s_l(&required, nullptr, 0, std::data(wcs), 0, locale); EILSEQ == ret) {
		throw std::invalid_argument("Invalid wide character sequence.");
	}

	// 変換に必要な出力バッファを確保する
	std::string buffer(required, '\0');

	size_t converted = 0;
	_wcstombs_s_l(&converted, std::data(buffer), std::size(buffer), std::data(wcs), _TRUNCATE, locale);

	buffer.resize(converted - 1); // wcstombs_sの戻り値は終端NULを含むので -1 する

	return buffer;
}

} // end of namespace cxx_util

namespace basis {

/*!
 * メッセージエラー
 * 
 * ワイド文字列でインスタンス化できるエラー。
 */
message_error::message_error(std::wstring_view message)
	: std::runtime_error(cxx_util::wcstombs_s(message))
	, _Message(message)
{
}

} // namespace basis

/*!
 * @brief コマンドライン引数を結合する
 */
static std::wstring CombineArg(const std::wstring& a, const std::wstring& b)
{
	LPCWSTR pszFormat = LR"(%s %s)";
	if (!std::regex_match(b, std::wregex(LR"(^.*\"$)")) && b.find(L" <>|()&") != std::wstring::npos) {
		pszFormat = LR"(%s "%s")";	// 引用符で囲む
	}
	return strprintf(pszFormat, a.c_str(), b.c_str());
}

/*!
 * @brief 新しいプロセスを起動する
 *
 * @return 起動したプロセスのスレッドID
 */
/* static */ DWORD CProcess::StartSakuraProcess(
	_In_opt_z_ LPCWSTR pszProfileName,
	const std::vector<std::wstring>& args,
	_In_opt_z_ LPCWSTR pszCurDir
)
{
	// コマンドライン文字列を構築する
	std::wstring command;

	// 実行可能モジュールのパスを取得する
	const auto exePath = GetExeFileName();

	// コマンドラインの先頭に実行可能モジュールのパスを追加する
	if (const auto path = exePath.wstring(); path.find(L" <>|()&") != std::wstring::npos) {
		command = strprintf(LR"("%s")", path.c_str());	// 引用符で囲む
	} else {
		command = path;
	}

	// プロファイル指定があればコマンドラインに追加する（空文字も「あり」に含める）
	if (pszProfileName) {
		command += strprintf(LR"( -PROF="%s")", pszProfileName);
	}

	// 残りのパラメーターをコマンドラインに追加する
	command += std::accumulate(args.cbegin(), args.cend(), std::wstring(), &CombineArg);

	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;

	// システムディレクトリ（コントロールプロセス用）
	SFilePath szSysDir;

	// ワーキングディレクトリ
	if (pszCurDir) {
		auto curDir = std::filesystem::path(pszCurDir);
		if (std::error_code ec; !std::filesystem::is_directory(curDir, ec)) {
			pszCurDir = nullptr;
		}
	}

	// プロセスのタイトル（コントロールプロセス用）
	SFilePath szTitle = L"sakura control process";

	// スタートアップ情報
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_FORCEONFEEDBACK;

	// プロセス情報
	PROCESS_INFORMATION pi{};

	if (std::find(args.begin(), args.end(), L"-NOWIN") != args.end()) {
		// システムディレクトリを取得する
		GetSystemDirectoryW(szSysDir, UINT(std::size(szSysDir)));
		pszCurDir = szSysDir;

		si.dwFlags |= STARTF_PREVENTPINNING;	// ピン留めを防止
		si.lpTitle = szTitle;

	} else {
		si.dwFlags |= STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWDEFAULT;
	}

	// プロセスを起動する
	if (!CreateProcessW(
		exePath.c_str(),	// 実行可能モジュールパス
		command.data(),		// コマンドラインバッファ
		nullptr,			// プロセスのセキュリティ記述子(カレントプロセスと同じ)
		nullptr,			// スレッドのセキュリティ記述子(カレントスレッドと同じ)
		FALSE,				// ハンドルの継承オプション(継承させない)
		dwCreationFlag,		// 作成のフラグ
		nullptr,			// 環境変数(変更しない)
		pszCurDir,			// カレントディレクトリ
		&si,				// スタートアップ情報
		&pi					// プロセス情報(作成されたプロセス情報を格納する構造体)
	))
	{
		// 失敗時、エラー理由をシステムから取得する
		LPWSTR pMsg = nullptr;
		FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_IGNORE_INSERTS
			,
			nullptr,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			LPWSTR(&pMsg),
			NULL,
			nullptr
		);

		std::wstring msg(pMsg);
		LocalFree(HLOCAL(pMsg));	//	エラーメッセージバッファを解放

		// "'%s'\nプロセスの起動に失敗しました。\n%s"
		throw basis::message_error(strprintf(LS(STR_TRAY_CREATEPROC1), exePath.c_str(), msg.c_str()));
	}

	// 開いたハンドルは使わないので閉じておく
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return pi.dwThreadId;
}

/*!
 * @brief 同期オブジェクトをシグナル状態にする
 */
/* static */ bool CProcess::SetSyncEvent()
{
	const auto dwThreadId = GetCurrentThreadId();
	SFilePath szEventName = strprintf(L"SakuraThread-0x%08x", dwThreadId);
	if (HandleHolder hEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, szEventName)) {
		return SetEvent(hEvent);
	}
	return false;
}

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
*/
CProcess::CProcess(
	HINSTANCE	hInstance,		//!< handle to process instance
	LPCWSTR		lpCmdLine		//!< pointer to command line
)
: m_hInstance( hInstance )
, m_hWnd( nullptr )
	, m_cShareData(CCommandLine::getInstance() ? CCommandLine::getInstance()->GetProfileName() : nullptr)
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
	@brief プロセスを初期化する

	共有メモリを初期化する
*/
bool CProcess::InitializeProcess()
{
	/* 共有データ構造体のアドレスを返す */
	if( !GetShareData().InitShareData() ){
		//	適切なデータを得られなかった
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONERROR,
			GSTR_APPNAME, L"異なるバージョンのエディタを同時に起動することはできません。" );
		return false;
	}

	/* リソースから製品バージョンの取得 */
	//	2004.05.13 Moca 共有データのバージョン情報はコントロールプロセスだけが
	//	ShareDataで設定するように変更したのでここからは削除

	return true;
}

/*!
	@brief プロセス実行
	
	@author aroka
	@date 2002/01/16
*/
bool CProcess::Run()
{
	if( InitializeProcess() )
	{
			MainLoop() ;
			OnExitProcess();
		return true;
	}
	return false;
}

/*!
	言語選択後に共有メモリ内の文字列を更新する
*/
void CProcess::RefreshString()
{
	m_cShareData.RefreshString();
}
