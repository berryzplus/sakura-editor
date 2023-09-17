/*!	@file
	@brief ウィンドウ一覧ダイアログボックス

	@author Moca
	@date 2015.03.07 Moca CDlgWindowList.cppを元に作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK, Moca
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, MIK, genta, じゅうじ
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2015, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgWindowList.h"
#include "Funccode_enum.h"
#include "util/shell.h"
#include "util/window.h"
#include "apiwrap/StdApi.h"
#include "debug/Debug2.h"
#include "util/string_ex.h"
#include "env/CAppNodeManager.h"
#include "env/DLLSHAREDATA.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "config/system_constants.h"

struct EditInfo;

const DWORD p_helpids[] = {
	IDC_LIST_WINDOW,			HIDC_WINLIST_LIST_WINDOW,
	IDC_BUTTON_SAVE,			HIDC_WINLIST_BUTTTN_SAVE,
	IDC_BUTTON_CLOSE,			HIDC_WINLIST_BUTTTN_CLOSE,
	IDOK,						HIDC_WINLIST_IDOK,
	0, 0
};

static const SAnchorList anchorList[] = {
	{IDC_LIST_WINDOW,			ANCHOR_ALL},
	{IDC_BUTTON_SAVE,			ANCHOR_BOTTOM},
	{IDC_BUTTON_CLOSE,			ANCHOR_BOTTOM},
	{IDOK,                      ANCHOR_BOTTOM},
	{IDC_BUTTON_HELP,           ANCHOR_BOTTOM},
};

CDlgWindowList::CDlgWindowList(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_)
	: CSizeRestorableDialog(IDD_WINLIST, std::move(ShareDataAccessor_))
{
	/* サイズ変更時に位置を制御するコントロール数 */
	assert(_countof(anchorList) == _countof(m_rcItems));
	return;
}

/* モーダルダイアログの表示 */
int CDlgWindowList::DoModal(
	HINSTANCE			hInstance,
	HWND				hwndParent,
	LPARAM				lParam
)
{
	return (int)CDialog::DoModal(hInstance, hwndParent, IDD_WINLIST, lParam);
}

BOOL CDlgWindowList::OnBnClicked(int wID)
{
	switch(wID){
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DLGWINLIST));
		return TRUE;
	case IDC_BUTTON_SAVE:
		CommandSave();
		return TRUE;
	case IDC_BUTTON_CLOSE:
		CommandClose();
		return TRUE;
	case IDOK:
		::EndDialog(GetHwnd(), TRUE);
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	return CDialog::OnBnClicked(wID);
}

void CDlgWindowList::GetDataListView(std::vector<HWND>& aHwndList)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_WINDOW);
	aHwndList.clear();
	const int nCount = ListView_GetItemCount(hwndList);
	for (int i = 0; i < nCount; i++) {
		const BOOL bCheck = ListView_GetCheckState(hwndList, i);
		if (bCheck) {
			LV_ITEM lvitem;
			memset_raw(&lvitem, 0, sizeof(lvitem));
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = i;
			lvitem.iSubItem = 0;
			if (ListView_GetItem(hwndList, &lvitem )) {
				aHwndList.push_back((HWND)lvitem.lParam);
			}
		}
	}
}

void CDlgWindowList::CommandSave()
{
	std::vector<HWND> aHwndList;
	GetDataListView(aHwndList);
	for (int i = 0; i < (int)aHwndList.size(); i++) {
		DWORD dwPid;
		::GetWindowThreadProcessId(aHwndList[i], &dwPid);
		::AllowSetForegroundWindow(dwPid);
		::SendMessage(aHwndList[i], WM_COMMAND, MAKELONG(F_FILESAVE, 0), 0);
	}
	SetData();
}

void CDlgWindowList::CommandClose()
{
	std::vector<HWND> aHwndList;
	GetDataListView(aHwndList);
	for (int i = 0; i < (int)aHwndList.size(); i++) {
		DWORD dwPid;
		::GetWindowThreadProcessId(aHwndList[i], &dwPid);
		::AllowSetForegroundWindow(dwPid);
		::SendMessage(aHwndList[i], MYWM_CLOSE, 0, 0);
	}
	SetData();
}

void CDlgWindowList::SetData()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_WINDOW);
	ListView_DeleteAllItems(hwndList);
	EditNode *pEditNode;
	int nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNode, TRUE, FALSE, GetShareDataAccessor());
	if (0 < nRowNum) {
		CTextWidthCalc calc(hwndList);
		for (int i = 0; i < nRowNum; i++) {
			::SendMessageAny(pEditNode[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
			const EditInfo* pEditInfo = &m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

			WCHAR szName[512];
			CFileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape(szName, _countof(szName), pEditInfo, pEditNode[i].m_nId, i, calc.GetDC());

			LV_ITEM lvi;
			lvi.mask     = LVIF_TEXT | LVIF_PARAM;
			lvi.pszText  = szName;
			lvi.iItem    = i;
			lvi.iSubItem = 0;
			lvi.lParam   = (LPARAM)pEditNode[i].GetHwnd();
			ListView_InsertItem(hwndList, &lvi);
		}

		delete [] pEditNode;
	}
	return;
}

int CDlgWindowList::GetData()
{
	return TRUE;
}

LPVOID CDlgWindowList::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}

/*!
 * WM_INITDIALOGハンドラ
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] wParam フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE  フォーカスを設定する
 * @retval FALSE フォーカスを設定しない
 */
BOOL CDlgWindowList::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	// 基底クラスのハンドラを呼び出す。
	const auto ret = __super::OnInitDialog(hwndDlg, wParam, lParam);

	for (int i = 0; i < _countof(anchorList); i++) {
		GetItemClientRect(anchorList[i].id, m_rcItems[i]);
	}

	if (const auto& rcDialog = GetShareData()->m_Common.m_sOthers.m_rcWindowListDialog;
		rcDialog.left != 0 || rcDialog.bottom != 0)
	{
		m_xPos    = rcDialog.left;
		m_yPos    = rcDialog.top;
		m_nWidth  = rcDialog.right  - rcDialog.left;
		m_nHeight = rcDialog.bottom - rcDialog.top;
	}

	HWND hwndList = GetItemHwnd(IDC_LIST_WINDOW);
	RECT rcListView;
	GetItemClientRect(IDC_LIST_WINDOW, rcListView);

	LV_COLUMN	col;
	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = rcListView.right - rcListView.left - ::GetSystemMetrics(SM_CXVSCROLL) - 10;
	WCHAR szNull[] = L"";
	col.pszText  = szNull;
	col.iSubItem = 0;
	ListView_InsertColumn(hwndList, 0, &col);
	LONG lngStyle = ListView_GetExtendedListViewStyle(hwndList);
	lngStyle |= LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
	ListView_SetExtendedListViewStyle(hwndList, lngStyle);

	::SetForegroundWindow(hwndDlg);
	::BringWindowToTop(hwndDlg);

	return ret;
}

/*!
 * WM_DESTROYハンドラ
 *
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
BOOL CDlgWindowList::OnDestroy( void )
{
	auto& rcDialog  = GetShareData()->m_Common.m_sOthers.m_rcWindowListDialog;
	rcDialog.left   = m_xPos;
	rcDialog.top    = m_yPos;
	rcDialog.right  = rcDialog.left + m_nWidth;
	rcDialog.bottom = rcDialog.top  + m_nHeight;
	return __super::OnDestroy();
}

BOOL CDlgWindowList::OnSize(WPARAM wParam, LPARAM lParam)
{
	CDialog::OnSize(wParam, lParam);

	RECT  rc;
	POINT ptNew;
	::GetWindowRect(GetHwnd(), &rc);
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for (int i = 0; i < _countof(anchorList); i++) {
		ResizeItem(GetItemHwnd(anchorList[i].id), m_ptDefaultSize, ptNew, m_rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}

/*!
 * WM_GETMINMAXINFOハンドラ
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [out] lpMinMaxInfo サイズ情報
 */
void CDlgWindowList::OnGetMinMaxInfo(HWND hDlg, _In_ LPMINMAXINFO lpMinMaxInfo)
{
	UNREFERENCED_PARAMETER(hDlg);

	lpMinMaxInfo->ptMinTrackSize.x = m_ptDefaultSize.x;
	lpMinMaxInfo->ptMinTrackSize.y = m_ptDefaultSize.y;
	lpMinMaxInfo->ptMaxTrackSize.x = m_ptDefaultSize.x * 3;
	lpMinMaxInfo->ptMaxTrackSize.y = m_ptDefaultSize.y * 3;
}

BOOL CDlgWindowList::OnActivate(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		SetData();
		return TRUE;
	case WA_INACTIVE:
	default:
		break;
	}
	return CDialog::OnActivate(wParam, lParam);
}
