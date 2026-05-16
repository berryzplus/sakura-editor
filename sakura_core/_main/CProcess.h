/*!	@file
	@brief プロセス基底クラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#define SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "_main/global.h"
#include "_main/CCommandLine.h"
#include "config/system_constants.h"
#include "env/CShareData.h"
#include "util/design_template.h"

namespace cxx {

/*!
 * @brief システムエラーを例外として発生させる
 *
 * @param message 追加のエラーメッセージ
 * @throw std::system_error システムエラー例外
 * @note 使い物になるかどうか試作してみただけ
 */
inline NORETURN void raise_system_error(const std::string& message) {
	throw std::system_error(int(::GetLastError()), std::system_category(), message);
}

/*!
 * @brief トップレベルウインドウを検索する
 */
inline HWND FindWindowW(std::wstring_view className, const std::optional<std::wstring>& optWindowName = std::nullopt)
{
	return ::FindWindowW(std::data(std::wstring(className)), optWindowName.has_value() ? std::data(*optWindowName) : nullptr);
}

/*!
 * @brief システムディレクトリのパスを取得する
 *
 * @return システムディレクトリのパス
 */
inline std::filesystem::path GetSystemDirectoryW()
{
	SFilePath buf;
	::GetSystemDirectoryW(buf, int(std::size(buf)));
	return LPCWSTR(buf);
}

//! HANDLE型のスマートポインタ
class HandleHolder : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = HandleHolder;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::ResourceHolder;

	void lock() const noexcept
	{
		Lock(INFINITE);	//無限に待つ
	}

	bool try_lock() const noexcept
	{
		return Lock(0);	//ロック取得を試行
	}

	template<class Rep, class Period>
	bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time) const noexcept
	{
		const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(rel_time);
		return Lock(DWORD(milliseconds.count()));
	}

	bool Lock(DWORD dwTimeout = INFINITE) const noexcept
	{
		// ロック取得を試行
		const auto dwRet = ::WaitForSingleObject(get(), dwTimeout);

		return WAIT_OBJECT_0 == dwRet || WAIT_ABANDONED == dwRet;
	}
};

class ProcessHolder : public cxx::HandleHolder
{
private:
	using Base = cxx::HandleHolder;
	using Me = ProcessHolder;

public:
	explicit ProcessHolder(
		HANDLE hProcess,
		DWORD dwProcessId,
		DWORD dwThreadId
	)
		: Base(hProcess)
		, dwProcessId(dwProcessId)
		, dwThreadId(dwThreadId)
	{
	}

	DWORD dwProcessId;
	DWORD dwThreadId;
};

class COleInit final
{
private:
	using Me = COleInit;

	bool m_initialized = false;

public:
	COleInit() noexcept
		: m_initialized(SUCCEEDED(::OleInitialize(nullptr)))
	{
	}

	COleInit(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	~COleInit() noexcept {
		if (m_initialized) ::OleUninitialize();
	}

	explicit operator bool() const noexcept {
		return m_initialized;
	}
};

class CComInit final
{
private:
	using Me = CComInit;

	bool m_initialized = false;

public:
	CComInit() noexcept
		: m_initialized(SUCCEEDED(::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
	{
	}

	CComInit(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	~CComInit() noexcept {
		if (m_initialized) ::CoUninitialize();
	}

	explicit operator bool() const noexcept {
		return m_initialized;
	}
};

} // namespace cxx

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス基底クラス
*/
class CProcess : public TSingleInstance<CProcess> {
private:
	using CCommandLineHolder = std::unique_ptr<CCommandLine>;

	using Base = TSingleInstance<CProcess>;
	using Me = CProcess;

public:
	/*!
	 * @brief サクラエディタのプロセスを起動する
	 *
	 * @tparam T コマンドライン引数のコンテナ型
	 * @param optFilePath ファイルパス（省略した場合は指定なし）
	 * @param args コマンドライン引数
	 * @param si スタートアップ情報
	 * @param optWorkingDir カレントディレクトリ（省略した場合は起動元と同じ）
	 * @param optProfileName プロファイル名（省略した場合は指定なし）
	 * @return 起動したプロセスのハンドルオブジェクト
	 * @note 使い物になるかどうか試作してみた
	 */
	template<class T>
		requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
	static cxx::ProcessHolder CreateSakuraProcess(
		const std::optional<std::filesystem::path>& optFilePath,
		const T& args,
		STARTUPINFO& si,
		const std::optional<std::filesystem::path>& optWorkingDir = std::nullopt,
		const std::optional<std::wstring>& optProfileName = std::nullopt
	)
	{
		const auto exePath = GetExeFileName();

		auto strCommandLine = std::format(LR"("{}")", exePath.native());

		if (optFilePath.has_value()) {
			strCommandLine += std::format(LR"( "{}")", optFilePath->native());
		}

		if (const auto pCommandLine = CCommandLine::getInstance(); optProfileName.has_value() || pCommandLine->IsSetProfile()) {
			strCommandLine += std::format(LR"( -PROF="{}")", optProfileName.value_or(GetProfileName()));
		}

		strCommandLine = std::accumulate(std::begin(args), std::end(args), strCommandLine, [](const std::wstring& a, std::wstring_view b) { return std::format(LR"({} {})", a, b); });

		auto lpszCommandLine = std::data(strCommandLine);

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
			exePath.c_str(),	// 実行可能モジュールパス
			lpszCommandLine,	// コマンドラインバッファ
			nullptr,			// プロセスのセキュリティ記述子
			nullptr,			// スレッドのセキュリティ記述子
			FALSE,				// ハンドルの継承オプション(継承させない)
			dwCreationFlag,		// 作成のフラグ
			nullptr,			// 環境変数(変更しない)
			lpszWorkingDir,		// カレントディレクトリ(変更しない)
			&si,				// スタートアップ情報
			&pi					// プロセス情報(作成されたプロセス情報を格納する構造体)
		))
		{
			cxx::raise_system_error("create process failed.");
		}

		// 開いたハンドルは使わないので閉じておく
		::CloseHandle(pi.hThread);

		return cxx::ProcessHolder(pi.hProcess, pi.dwProcessId, pi.dwThreadId);
	}

	/*!
	 * @brief コントロールプロセスを起動する
	 *
	 * @param profileName プロファイル名
	 * @return コントロールプロセスのプロセスID
	 */
	static DWORD CreateControlProcess(
		const std::optional<std::wstring>& optProfileName = std::nullopt
	)
	{
		// 初期化完了イベントの名前を決める
		SFilePath initEventName{ GSTR_EVENT_SAKURA_CP_INITIALIZED };
		if (optProfileName.has_value()) {
			initEventName += *optProfileName;
		}

		// プロセス起動前に初期化完了イベントを作成する
		cxx::HandleHolder hEvent = ::CreateEventW(nullptr, TRUE, FALSE, initEventName);
		if (!hEvent) {
			cxx::raise_system_error("create event failed.");
		}

		std::wstring title{ L"sakura control process" };

		// スタートアップ情報（入力用構造体なので値を入れる）
		STARTUPINFO si = { sizeof(STARTUPINFO) };
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.lpTitle = std::data(title);
		si.wShowWindow = SW_SHOWDEFAULT;

		// コントロールプロセスを起動する
		const auto cp = CreateSakuraProcess(std::nullopt, std::array{ LR"(-NOWIN)" }, si, cxx::GetSystemDirectoryW(), optProfileName);

		// 初期化完了を待つ
		if (!hEvent.try_lock_for(std::chrono::milliseconds(60000))){
			cxx::raise_system_error("waitEvent is timeout.");
		}

		// プロセスIDを返す
		return cp.dwProcessId;
	}

	static BOOL CALLBACK MyEnumThreadWndProc(
		_In_ HWND   hWnd,
		_In_ LPARAM lParam
	)
	{
		auto phWndFound = std::bit_cast<HWND*>(lParam);

		*phWndFound = hWnd;

		return FALSE;
	}

	/*!
	 * @brief エディタープロセスを起動する
	 *
	 * @tparam T コマンドライン引数のコンテナ型
	 * @param optFilePath ファイルパス（省略した場合は指定なし）
	 * @param args コマンドライン引数
	 * @param profileName プロファイル名
	 * @return 起動したプロセスのハンドルオブジェクト
	 * @note 使い物になるかどうか試作してみた
	 */
	template<class T>
		requires std::ranges::range<T> && std::convertible_to<std::ranges::range_reference_t<T>, std::wstring_view>
	static cxx::ProcessHolder CreateEditorProcess(
		const std::optional<std::filesystem::path>& optFilePath,
		const T& args,
		const std::optional<std::wstring>& optProfileName = std::nullopt,
		const std::optional<std::filesystem::path>& optWorkingDir = std::nullopt
	)
	{
		// スタートアップ情報（入力用構造体なので値を入れる）
		STARTUPINFO si = { sizeof(STARTUPINFO) };
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWDEFAULT;

		const auto optFileDir = optFilePath.has_value() ? std::optional(std::filesystem::path(*optFilePath).remove_filename()) : std::nullopt;

		// エディタープロセスを起動する
		auto ep = CreateSakuraProcess(optFilePath, args, si, optWorkingDir.has_value() ? optWorkingDir : optFileDir, optProfileName);
		if (!ep) return ep;

		HWND hWndFound = nullptr;

		const auto startTick = ::GetTickCount64();

		// メインウインドウを取得する
		do {
			// スレッドに含まれるウインドウを列挙する
			::EnumThreadWindows(ep.dwThreadId, MyEnumThreadWndProc, LPARAM(&hWndFound));

			// ウィンドウがVisibleかつEnabledになるのを待つ
			if (hWndFound &&
				::IsWindowEnabled(hWndFound) &&
				::IsWindowVisible(hWndFound))
			{
				break;
			}

			Sleep(10);  // 10msスリープしてリトライ
		}
		while (::GetTickCount64() - startTick < 5000);

		return ep;
	}

	CProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine);
	~CProcess() override = default;

	int		Run(int nCmdShow);

	virtual void RefreshString();

	virtual std::filesystem::path GetIniFileName() const;

protected:
	virtual bool	InitializeProcess(int nCmdShow) = 0;

	virtual int		MainLoop() const { return 0L; }

	void			SetMainWindow(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		GetProcessInstance() const{ return m_hInstance; }
	CShareData&		GetShareData()   { return m_cShareData; }
	HWND			GetMainWindow() const{ return m_hWnd; }

	[[nodiscard]] const CShareData* GetShareDataPtr() const { return &m_cShareData; }

private:
	HINSTANCE	m_hInstance;

	CCommandLineHolder m_pCommandLine;

	HWND		m_hWnd = nullptr;
	CShareData		m_cShareData;
};

#endif /* SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_ */
