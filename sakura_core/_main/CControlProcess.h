/*!	@file
	@brief コントロールプロセスクラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_
#define SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_
#pragma once

#include "_main/CControlTray.h"
#include "_main/CProcess.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

namespace cxx {

struct MutexHolder final : public cxx::HandleHolder {
	using MutexReleaser = cxx::ResourceHolder<&::ReleaseMutex>;

	using Base = cxx::HandleHolder;
	using Me = MutexHolder;

	MutexReleaser m_Releaser;

	explicit MutexHolder(HANDLE hMutex)
		: HandleHolder(hMutex)
		, m_Releaser(hMutex)
	{
	}

	Me& operator = (HANDLE t)
	{
		HandleHolder::operator=(t);
		m_Releaser = t;
		return *this;
	}
};

struct EventHolder final : public cxx::HandleHolder {
	using EventReleaser = cxx::ResourceHolder<&::ResetEvent>;

	using Base = cxx::HandleHolder;
	using Me = EventHolder;

	EventReleaser m_Releaser;

	explicit EventHolder(HANDLE hEvent)
		: HandleHolder(hEvent)
		, m_Releaser(hEvent)
	{
	}

	Me& operator = (HANDLE t)
	{
		HandleHolder::operator=(t);
		m_Releaser = t;
		return *this;
	}
};

} // namespace cxx

/*!
	@brief コントロールプロセスクラス
	
	コントロールプロセスはCControlTrayクラスのインスタンスを作る。
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CControlProcess final : public CProcess {
private:
	using CComInitHolder = std::unique_ptr<cxx::CComInit>;

	using Base = CProcess;
	using Me = CControlProcess;

public:
	using Base::Base;

	~CControlProcess() override = default;

	std::filesystem::path GetIniFileName() const override;

private:
	bool	InitializeProcess(int nCmdShow [[maybe_unused]]) override;
	bool	MainLoop() override;

	std::filesystem::path GetPrivateIniFileName(const std::wstring& exeIniPath, const std::wstring& filename) const;

	CComInitHolder		m_ComInit = std::make_unique<cxx::CComInit>();

	cxx::MutexHolder	m_hMutex{ nullptr };				//!< アプリケーション実行検出用ミューテックス
	cxx::MutexHolder	m_hMutexCP{ nullptr };				//!< コントロールプロセスミューテックス
	cxx::EventHolder	m_hEventCPInitialized{ nullptr };	//!< コントロールプロセス初期化完了イベント 2006.04.10 ryoji

	std::unique_ptr<CControlTray>	m_pcTray = nullptr;
};

#endif /* SAKURA_CCONTROLPROCESS_AFB90808_4287_4A11_B7FB_9CD21CF8BFD6_H_ */
