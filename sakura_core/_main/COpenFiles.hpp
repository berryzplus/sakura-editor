/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "_main/CProcess.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 複数ファイルオープン
	
	共有メモリを初期化し、指定された複数ファイルを開く。
	開き終わったらプロセスは終了する。
*/
class COpenFiles final : public CProcess {
private:
	using Base = CProcess;
	using Me = COpenFiles;

public:
	// コンストラクタは流用する
	using Base::Base;

private:
	//プロセスハンドラ
	bool InitializeProcess(
		int nCmdShow [[maybe_unused]]
	) override;

	//実装補助
	void	OpenFiles() const;
};
