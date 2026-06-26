/*!	@file
	@brief ウィンドウの基本クラス

	@author Norio Nakatani
	@date 2000/01/11 新規作成
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "window/CWnd.h"

CWnd::CWnd()
{
	return;
}

/*!
 * @brief ウィンドウのメッセージ配送
 *
 * @param hWnd [in] 宛先ウインドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CWnd::DispatchEvent(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (uMsg) {
// clang-format off
	HANDLE_MSG(hWnd, WM_DESTROY,						OnDestroy);
	HANDLE_MSG(hWnd, WM_SIZE,							OnSize);
	HANDLE_MSG(hWnd, WM_DRAWITEM,						OnDrawItem);
	HANDLE_MSG(hWnd, WM_MEASUREITEM,					OnMeasureItem);
	HANDLE_MSG(hWnd, WM_COMMAND,						OnCommand);
	HANDLE_MSG(hWnd, WM_TIMER,							OnTimer);
	HANDLE_MSG(hWnd, WM_MOUSEMOVE,						OnMouseMove);
	HANDLE_MSG(hWnd, WM_LBUTTONDOWN,					OnLButtonDown);
	HANDLE_MSG(hWnd, WM_LBUTTONUP,						OnLButtonUp);
	HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK,					OnLButtonDown);
	HANDLE_MSG(hWnd, WM_RBUTTONDOWN,					OnRButtonDown);
	HANDLE_MSG(hWnd, WM_MBUTTONDOWN,					OnMButtonDown);
// clang-format on

	case WM_PAINT:
		// 描画処理を開始する
		if (PAINTSTRUCT ps; ::BeginPaint(hWnd, &ps)) {
			// 更新領域が空でない場合、描画を行う
			OnPaint(hWnd, ps);

			// 描画処理を終了する
			::EndPaint(hWnd, &ps);
		}
		return 0L;

	case WM_NOTIFY:
		if (auto pNMHDR = LPNMHDR(lParam)) {
			return OnNotify(hWnd, pNMHDR->idFrom, pNMHDR);
		}
		break;

	default:
		break;
	}

	//あとはデフォルトに任せる
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief WM_DESTROYハンドラ
 *
 * WM_DESTROYはDestroyWindow関数によるウインドウ破棄中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CWnd::OnDestroy(HWND hWnd)
{
	FORWARD_WM_DESTROY(hWnd, DefWndProcW);
}

/*!
 * @brief WM_SIZEハンドラ
 *
 * WM_SIZEはWM_WINDOWPOSCHANGEDの処理中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CWnd::OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	FORWARD_WM_SIZE(hWnd, state, cx, cy, DefWndProcW);
}

/*!
 * @brief WM_PAINTハンドラ
 *
 * WM_PAINTはウィンドウの描画中にポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 * @note windowsx.h の定義が微妙なので独自に定義
 */
void CWnd::OnPaint(HWND hWnd, PAINTSTRUCT& ps)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(ps);

	// 何もしない
}

void CWnd::OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT* lpDrawItem)
{
	FORWARD_WM_DRAWITEM(hWnd, lpDrawItem, DefWndProcW);
}

void CWnd::OnMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* lpMeasureItem)
{
	FORWARD_WM_MEASUREITEM(hWnd, lpMeasureItem, DefWndProcW);
}

/*!
 * @brief WM_NOTIFYハンドラ
 *
 * WM_NOTIFYは子ウィンドウからポストされます。
 *
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CWnd::OnNotify(HWND hWnd, UINT_PTR idFrom, LPNMHDR pNMHDR)
{
	return FORWARD_WM_NOTIFY(hWnd, idFrom, pNMHDR, DefWndProcW);
}

/*!
 * @brief WM_COMMANDハンドラ
 *
 * WM_COMMANDは子孫ウィンドウからポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CWnd::OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT notifyCode)
{
	FORWARD_WM_COMMAND(hWnd, id, hWndCtl, notifyCode, DefWndProcW);
}

/*!
 * @brief WM_TIMERハンドラ
 *
 * WM_TIMERはSetTimer関数で作成したタイマーからポストされます。
 *
 * @returns このメッセージに戻り値はありません。
 */
void CWnd::OnTimer(HWND hWnd, UINT id)
{
	FORWARD_WM_TIMER(hWnd, id, DefWndProcW);
}

void CWnd::OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
	FORWARD_WM_MOUSEMOVE(hWnd, x, y, keyFlags, DefWndProcW);
}

void CWnd::OnLButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	if (fDoubleClick) {
		OnLButtonDblClk(hWnd, x, y, keyFlags);
		return;
	}

	FORWARD_WM_LBUTTONDOWN(hWnd, FALSE, x, y, keyFlags, DefWndProcW);
}

void CWnd::OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
	FORWARD_WM_LBUTTONUP(hWnd, x, y, keyFlags, DefWndProcW);
}

void CWnd::OnLButtonDblClk(HWND hWnd, int x, int y, UINT keyFlags)
{
	FORWARD_WM_LBUTTONDOWN(hWnd, TRUE, x, y, keyFlags, DefWndProcW);
}

void CWnd::OnRButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	FORWARD_WM_RBUTTONDOWN(hWnd, fDoubleClick, x, y, keyFlags, DefWndProcW);
}

void CWnd::OnMButtonDown(HWND hWnd, bool fDoubleClick, int x, int y, UINT keyFlags)
{
	FORWARD_WM_MBUTTONDOWN(hWnd, fDoubleClick, x, y, keyFlags, DefWndProcW);
}

/*!
 * @brief Windowsと直接やり取りするコールバックプロシージャ
 *
 * @param hWnd [in] 宛先ウインドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @param uIdSubclass [in] サブクラスID
 * @param dwRefData [in] サブクラスに関連付けられたユーザーデータ(CWndポインター。)
 * @returns 処理結果 メッセージコードにより異なる
 */
/* static */ LRESULT CALLBACK CCustomizedWnd::SubclassProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	SLresult ret;
	if (auto pcWnd = std::bit_cast<CWnd*>(dwRefData)) {
		ret = SLresult(pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam));
	}

	if (WM_DESTROY == uMsg) {
		::RemoveWindowSubclass(hWnd, &SubclassProc, uIdSubclass);
	}

	if (ret) {
		return ret.result;
	}

	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

bool CCustomizedWnd::Attach(HWND hWnd, UINT uIdSubclass)
{
	const auto ret = ::SetWindowSubclass(hWnd, &SubclassProc, uIdSubclass, DWORD_PTR(this));
	if (!ret) return false;
	m_IdSubclass = uIdSubclass;
	return ret;
}

void CCustomizedWnd::Detach(HWND hWnd)
{
	if (const auto ret = ::RemoveWindowSubclass(hWnd, &SubclassProc, m_IdSubclass); !ret) return;
	m_IdSubclass = 0;
}

LRESULT CCustomizedWnd::DefWndProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
	return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief Windowsと直接やり取りするコールバックプロシージャ
 *
 * @param hWnd [in] 宛先ウインドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
/* static */ LRESULT CALLBACK COriginalWnd::WndProc(
	HWND	hWnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
) /* noexcept */
{
	// WM_CREATEが来たらウインドウに作成パラメーターを関連付ける
	if (auto lpCreateStruct = LPCREATESTRUCTW(lParam);
		WM_NCCREATE == uMsg &&
		lpCreateStruct &&
		lpCreateStruct->lpCreateParams)
	{
		// ウインドウ作成パラメーターには this ポインターが渡されている
		auto pcWnd = std::bit_cast<CWnd*>(lpCreateStruct->lpCreateParams);

		// ウインドウハンドルを関連付ける
		pcWnd->_SetHwnd(hWnd);

		// ウインドウハンドルにクラスオブジェクトを関連付ける
		::SetWindowLongPtrW(hWnd, GWLP_USERDATA, LONG_PTR(lpCreateStruct->lpCreateParams));

		return pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam);
	}

	// GetWindowLongPtr する都合、NULLを弾く
	if (!hWnd) {
		return 0L;
	}

	// ウインドウに関連付けられたオブジェクトに処理を委譲
	if (auto pcWnd = std::bit_cast<CWnd*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
		const auto ret = pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam);
		if (WM_NCDESTROY == uMsg) {
			// クラスオブジェクトの関連付けを解除する
			::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0L);

			// ウインドウハンドルの関連付けを解除する
			pcWnd->_SetHwnd(nullptr);
		}
		return ret;
	}

	//あとはデフォルトに任せる
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

COriginalWnd::~COriginalWnd()
{
	DestroyWindow();
	return;
}

/*!
 * @brief 独自ウィンドウのウィンドウクラスを登録する。
 *
 * @retval != 0 登録されたウィンドウクラスのATOM値。
 * @retval == 0 登録に失敗した。
 *
 * @note RegisterWCからRegisterClassWに改称。
 */
ATOM COriginalWnd::RegisterClassW(
	HBRUSH		hbrBackground,
	HCURSOR		hCursor,
	UINT		style,
	int			cbWndExtra,
	HICON		hIcon,
	LPCWSTR		lpszMenuName,
	HICON		hIconSm
) const
{
	const auto hInstance = G_AppInstance();

	WNDCLASSEX wc{ sizeof(WNDCLASSEX) };

	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style		 = style;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = cbWndExtra;
	wc.hInstance     = hInstance;
	wc.hIcon         = hIcon;
	wc.hCursor       = hCursor;
	wc.hbrBackground = hbrBackground;
	wc.lpszMenuName  = lpszMenuName;
	wc.lpszClassName = m_ClassName.c_str();
	wc.hIconSm       = hIconSm;

	//ウィンドウクラスの登録
	return ::RegisterClassExW(&wc);
}

/*!
 * @brief 独自ウィンドウを作成する。
 *
 * @retval != 0 作成されたウィンドウハンドル。
 * @retval == 0 作成に失敗した。
 *
 * @note CreateからCreateWndに改称。
 */
HWND COriginalWnd::CreateWnd(
	ATOM				atom,			//!< registered class atom, or pointer to a null-terminated string.
	DWORD				dwStyle,		//!< window style
	HWND				hWndParent,		//!< handle to parent window
	size_t				windowId,		//!< child-window identifier, or handle to menu for top-level windows
	const CMyRect&		rc,				//!< window rect
	const std::optional<std::wstring>& optCaption,
	DWORD				dwExStyle
) const
{
	auto pClassName = std::data(m_ClassName);
	if (atom) {
		pClassName = MAKEINTATOM(atom);
	}

	const auto caption = optCaption.value_or(L"");

	return ::CreateWindowExW(
		dwExStyle,				// extended window style
		pClassName,				// pointer to registered class name
		std::data(caption),		// pointer to window name
		dwStyle,				// window style
		rc.left,				// horizontal position of window
		rc.top,					// vertical position of window
		rc.Width(),				// window width
		rc.Height(),			// window height
		hWndParent,				// handle to parent or owner window
		HMENU(windowId),		// child-window identifier, or handle to menu for top-level windows
		G_AppInstance(),		// handle to application instance
		LPVOID(this)			// pointer to window-creation data
	);
}

/*!
 * @brief 独自ウィンドウのメッセージ配送
 *
 * @param hWnd [in] 宛先ウインドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT COriginalWnd::DispatchEvent(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	// 独自ウィンドウはWM_CREATEを処理する
	if (WM_CREATE == uMsg) {
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
	}

	// 0x8000 - 0xBFFF はアプリケーション定義のメッセージ
	if (WM_APP <= uMsg && uMsg <= WM_APP + 0x3FFF) {
		/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
		return DispatchEvent_WM_APP(hWnd, uMsg, wParam, lParam);
	}

	//あとはデフォルトに任せる
	return Base::DispatchEvent(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 *
 * @returns ウインドウの作成を続行してよいかどうか
 * @retval true  ウィンドウの作成を続行してよい
 * @retval false ウィンドウの作成を続行してはいけない（作成を中止する）
 */
bool COriginalWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	if (!hWnd || !lpCreateStruct) {
		return false;
	}

	m_hInstance = lpCreateStruct->hInstance;

	m_hwndParent = lpCreateStruct->hwndParent;

	return true;
}

/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
LRESULT COriginalWnd::DispatchEvent_WM_APP(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CallDefWndProc(hWnd, uMsg, wParam, lParam);
}

/* デフォルトメッセージ処理 */
LRESULT COriginalWnd::CallDefWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/*!
 * @brief ウィンドウを閉じる
 *
 * あちこちに実装されていたのを共通化したもの。
 */
void COriginalWnd::Close() const
{
	// ウィンドウを閉じる
	DestroyWindow();
}

/* ウィンドウを破棄 */
void COriginalWnd::DestroyWindow() const
{
	if (const auto hWnd = GetHwnd(); ::IsWindow(hWnd)) {
		::DestroyWindow(hWnd);
	}
}
