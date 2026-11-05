/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#include "StdAfx.h"
#include "window/CDropTargetWnd.hpp"

#include "_os/CDropTarget.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"

#include "CSelectLang.h"

/*!
 * @brief クリーンアップ関数
 */
/* static */ void CDropTargetWnd::_CleanUp(
	CDropTargetWnd* pThis
) noexcept
{
	if (pThis) pThis->RevokeDropTarget();
}

 /*!
 * @brief 拡張ウィンドウのプロシージャ
 *
 * @param hWnd [in] 宛先ウィンドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @param uIdSubclass [in] サブクラスID
 * @param dwRefData [in] サブクラスデータ
 *
 * @returns  メッセージ処理結果。値の意味はメッセージ依存。
 */
/* static */ LRESULT CALLBACK CDropTargetWnd::SubclassProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	// サブクラスデータを取り出す。（格納方法は改善検討要。）
	auto pcWnd = std::bit_cast<Me*>(dwRefData);

	if (WM_DESTROY == uMsg) {
		if (pcWnd) {
			pcWnd->RevokeDropTarget();
		}

		// 元々のハンドラを呼び出す
		FORWARD_WM_DESTROY(hWnd, ::DefSubclassProc);

		// ウィンドウのサブクラス化を解除する
		::RemoveWindowSubclass(hWnd, &SubclassProc, uIdSubclass);

		return 0;
	}

	//あとはデフォルトに任せる
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

template <typename T>
bool CDropTargetWnd::AttachDropTarget(
	T* pThis
)
{
	if constexpr (std::is_same_v<T, CEditWnd>)
	{
		m_pcDropTarget = CDropTarget_CEditWnd::make_instance();
	}
	else if constexpr (std::is_same_v<T, CEditView>)
	{
		m_pcDropTarget = CDropTarget_CEditView::make_instance(pThis);
	}

	// ウィンドウハンドルを取得する
	const auto hWnd = pThis->GetHwnd();

	// ウィンドウをサブクラス化する
	if (!::SetWindowSubclass(hWnd, &SubclassProc, 0, DWORD_PTR(this))) return false;

	// ウィンドウハンドルを紐付ける
	m_hWnd = hWnd;

	return RegisterDropTarget();
}

bool CDropTargetWnd::RegisterDropTarget()
{
	auto bRet = false;

	if (!m_hWnd_DropTarget &&
		FAILED(::RegisterDragDrop(m_hWnd, m_pcDropTarget)))
	{
		// L"::RegisterDragDrop()\n失敗"
		TopWarningMessage(m_hWnd, LS(STR_ERR_DLGDRPTGT1));
	}
	else {
		m_hWnd_DropTarget = m_hWnd;

		bRet = true;
	}

	return bRet;
}

void CDropTargetWnd::RevokeDropTarget() noexcept
{
	if (m_hWnd_DropTarget &&
		SUCCEEDED(::RevokeDragDrop(m_hWnd_DropTarget)))
	{
		m_hWnd_DropTarget = nullptr;
	}
}

template bool CDropTargetWnd::AttachDropTarget<CEditWnd>(CEditWnd* pThis);
template bool CDropTargetWnd::AttachDropTarget<CEditView>(CEditView* pThis);
