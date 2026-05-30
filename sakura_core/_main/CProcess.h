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

#include "_main/global.h"
#include "_main/CCommandLine.h"
#include "env/CShareData.h"
#include "util/design_template.h"
#include "util/tchar_convert.h"

namespace cxx {

/*!
 * @brief システムエラーを例外として発生させる
 *
 * @param message 追加のエラーメッセージ
 * @throw std::system_error システムエラー例外
 */
[[noreturn]] inline void raise_system_error(const std::string& message) { throw std::system_error(int(::GetLastError()), std::system_category(), message); }

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

/*!
 * @brief 起動したプロセスオブジェクト
 */
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

	ProcessHolder() = default;

	ProcessHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	ProcessHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	DWORD dwProcessId = 0;
	DWORD dwThreadId = 0;
};

/*!
 * @brief ロックしたミューテックスオブジェクト
 */
struct MutexHolder : public cxx::HandleHolder {
	using MutexReleaser = cxx::ResourceHolder<&::ReleaseMutex>;

	using Base = cxx::HandleHolder;
	using Me = MutexHolder;

	MutexReleaser m_Releaser;

	explicit MutexHolder(HANDLE hMutex)
		: HandleHolder(hMutex)
		, m_Releaser(hMutex)
	{
	}

	MutexHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	MutexHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	~MutexHolder() override = default;

	bool Lock(DWORD dwTimeout = INFINITE) override
	{
		if (!m_Releaser) {
			m_Releaser = get();
		}
		return Base::Lock(dwTimeout);
	}

	bool Unlock() override
	{
		m_Releaser = nullptr;
		return Base::Unlock();
	}

	void reset(HANDLE h)
	{
		Unlock();
		m_Holder.reset(h);
		m_Releaser = h;
	}

	Me& operator = (HANDLE h)
	{
		reset(h);
		return *this;
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
public:
	static cxx::ProcessHolder CreateSakuraProcess(
		STARTUPINFO& si,
		std::vector<std::wstring>& args,
		const std::optional<std::filesystem::path>& optWorkingDir = std::nullopt,
		const std::optional<std::wstring>& optProfileName = std::nullopt
	);

	static cxx::ProcessHolder CreateControlProcess(
		const std::optional<std::wstring>& optProfileName = std::nullopt
	);

	CProcess( HINSTANCE hInstance, LPCWSTR lpCmdLine );
	~CProcess() override = default;

	bool Run();
	virtual void RefreshString();

	virtual std::filesystem::path GetIniFileName() const;

protected:
	virtual bool	InitializeProcess() = 0;

	virtual bool MainLoop() = 0;
	virtual void OnExitProcess() = 0;

protected:
	void			SetMainWindow(HWND hwnd){ m_hWnd = hwnd; }

public:
	HINSTANCE		GetProcessInstance() const{ return m_hInstance; }
	CShareData&		GetShareData()   { return m_cShareData; }
	HWND			GetMainWindow() const{ return m_hWnd; }

	[[nodiscard]] const CShareData* GetShareDataPtr() const { return &m_cShareData; }

private:
	HINSTANCE	m_hInstance;
	HWND		m_hWnd = nullptr;
	CShareData		m_cShareData;
};

#endif /* SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_ */
