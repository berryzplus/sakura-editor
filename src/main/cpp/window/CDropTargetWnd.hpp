/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/com_pointer.hpp"

#include <oleidl.h>

/*!
 * @brief ドロップターゲットウィンドウ
 * 
 * CDropTargetに実装されていたものを分離。
 * CDropTargetはIDropTargetの実装に専念させる。
 */
class CDropTargetWnd
{
private:
	using DropTargetHolder = cxx::com_pointer<IDropTarget>;

	using Me = CDropTargetWnd;

	static void _CleanUp(
		CDropTargetWnd* pThis
	) noexcept;

	using Holder = cxx::ResourceHolder<&_CleanUp>;

	static LRESULT CALLBACK SubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		UINT_PTR uIdSubclass,
		DWORD_PTR dwRefData
	);

public:
	template <typename T>
	bool AttachDropTarget(
		T* pThis
	);

	bool RegisterDropTarget();
	void RevokeDropTarget() noexcept;

private:
	HWND				m_hWnd = nullptr;
	DropTargetHolder	m_pcDropTarget = nullptr;
	HWND				m_hWnd_DropTarget = nullptr;
	Holder				m_Holder{ this };
};
