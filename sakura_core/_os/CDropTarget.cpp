/*!	@file
	@brief Drag & Drop

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani, Yebisuya Sugoroku
	Copyright (C) 2002, aroka
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_os/CDropTarget.h"

#include "window/CEditWnd.h"	// 2008.06.20 ryoji
#include "view/CEditView.h"// 2002/2/3 aroka
#include "_main/global.h"
#include "CClipboard.h"
#include "CSelectLang.h"

COleLibrary CYbInterfaceBase::m_olelib;

CYbInterfaceBase::CYbInterfaceBase()
{
	m_olelib.Initialize();
	return;
}
CYbInterfaceBase::~CYbInterfaceBase()
{
	m_olelib.UnInitialize();
	return;
}

HRESULT CYbInterfaceBase::QueryInterfaceImpl(
	IUnknown*	pThis,
	REFIID		owniid,
	REFIID		riid,
	void**		ppvObj
)
{
	if( riid == IID_IUnknown || riid == owniid ){
		pThis->AddRef();
		*ppvObj = pThis;
		return S_OK;
	}
	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

/////////////////////////////////////////

COleLibrary::COleLibrary()//:m_dwCount(0)	// 2009.01.08 ryoji m_dwCount削除
{
	return;
}
COleLibrary::~COleLibrary()
{
// 2009.01.08 ryoji OleUninitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if( m_dwCount > 0 )
//		::OleUninitialize();
	return;
}

void COleLibrary::Initialize()
{
// 2009.01.08 ryoji OleInitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if( m_dwCount++ == 0 )
//		::OleInitialize( NULL );
	return;
}

void COleLibrary::UnInitialize()
{
// 2009.01.08 ryoji OleUninitialize削除（WinMainにOleInitialize/OleUninitialize追加）
//	if( m_dwCount > 0 && --m_dwCount == 0 )
//		::OleUninitialize();
	return;
}

#define DECLARE_YB_INTERFACEIMPL( BASEINTERFACE ) \
template<> REFIID CYbInterfaceImpl<BASEINTERFACE>::m_owniid = IID_##BASEINTERFACE;

IFACEMETHODIMP CDropTarget_CEditView::DragEnter(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
)
{
	return m_pcEditView->DragEnter( pDataObject, dwKeyState, pt, pdwEffect );
}

IFACEMETHODIMP CDropTarget_CEditView::DragOver(
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
)
{
	return m_pcEditView->DragOver( dwKeyState, pt, pdwEffect );
}

IFACEMETHODIMP CDropTarget_CEditView::DragLeave()
{
	return m_pcEditView->DragLeave();
}

IFACEMETHODIMP CDropTarget_CEditView::Drop(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
)
{
	return m_pcEditView->Drop( pDataObject, dwKeyState, pt, pdwEffect );
}

STDMETHODIMP CDropSource::QueryContinueDrag( BOOL bEscapePressed, DWORD dwKeyState )
{
	if( bEscapePressed || (dwKeyState & (m_bLeft ? MK_RBUTTON : MK_LBUTTON)) )
		return DRAGDROP_S_CANCEL;
	if( !(dwKeyState & (m_bLeft ? MK_LBUTTON : MK_RBUTTON)) )
		return DRAGDROP_S_DROP;
	return S_OK;
}

STDMETHODIMP CDropSource::GiveFeedback( [[maybe_unused]] DWORD dropEffect )
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

DWORD CDropSource::DoDragDrop(LPDATAOBJECT pDataObject, DWORD dwDesiredEffects)
{
	if (DWORD dwEffects; SUCCEEDED(::DoDragDrop(pDataObject, this, dwDesiredEffects, &dwEffects))) {
		return dwEffects;
	}

	return DROPEFFECT_NONE;
}

/* static */ UINT CDataObject::cfSakuraClipW	= ::RegisterClipboardFormatW(L"SAKURAClipW");
/* static */ UINT CDataObject::cfColumn		= ::RegisterClipboardFormatW(L"MSDEVColumnSelect");
/* static */ UINT CDataObject::cfLine1		= ::RegisterClipboardFormatW(L"MSDEVLineSelect");
/* static */ UINT CDataObject::cfLine2		= ::RegisterClipboardFormatW(L"VisualStudioEditorOperationsLineCutCopyClipboardTag");

/*!
 * @brief コンストラクター
 * 
 * @param[in] text クリップボードに格納する文字列
 * @param[in] bColumnSelect 矩形選択か
 * @param[in] bLineSelect 行選択か
 * @param[in] eCharset 文字コード種別
 * 
 * @date 2008/03/26 ryoji 複数フォーマット対応
*/
CDataObject::CDataObject(
	std::wstring_view text,
	bool bColumnSelect,
	bool bLineSelect,
	ECodeType eCharset
)
	: m_Text(text)
	, m_eCodeType(eCharset)
{
	// UNICODETEXTはデフォルトで作成する

	// 独自形式もできるだけ作成する
	if (m_Text.length() <= INT32_MAX) {
		// 長さフィールドで表せない長さのときは独自形式では受け渡さない
		m_Formats.emplace_back(cfSakuraClipW);
	}

	// 矩形選択が指定されている場合
	if (bColumnSelect) {
		m_Formats.emplace_back(cfColumn);
	}

	// 行選択が指定されている場合
	if (bLineSelect) {
		m_Formats.emplace_back(cfLine1);
		m_Formats.emplace_back(cfLine2);
	}

	// 文字コードがSJISの場合
	if (ECodeType::CODE_SJIS == m_eCodeType) {
		m_Formats.emplace_back(CF_TEXT);
		m_Formats.emplace_back(CF_LOCALE);
	}
}

/** IDataObject::GetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::GetData(
	LPFORMATETC lpfe,
	LPSTGMEDIUM lpsm
)
{
	// OLE_E_NOTRUNNING は返さない

	// 内部データは変更不可。

	if (!lpfe || !lpsm)
		return E_INVALIDARG;

	if (const auto hr = QueryGetData(lpfe); FAILED(hr)) return hr;

	FORMATETC fe = *lpfe;
	if (const auto hr = GetCanonicalFormatEtc(lpfe, &fe); FAILED(hr)) return hr;

	auto memory = cxx::GlobalData<BYTE>{ _CalcSize(fe.cfFormat) };

	lpsm->tymed = TYMED_HGLOBAL;
	lpsm->hGlobal = memory.release();
	lpsm->pUnkForRelease = nullptr;

	return GetDataHere(lpfe, lpsm);
}

/** IDataObject::GetDataHere
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::GetDataHere(
	LPFORMATETC lpfe,
	LPSTGMEDIUM lpsm
)
{
	// OLE_E_NOTRUNNING は返さない

	// 内部データは変更不可。

	if (!lpfe || !lpsm || !lpsm->hGlobal)
		return E_INVALIDARG;

	if (const auto hr = QueryGetData(lpfe); FAILED(hr)) return hr;

	FORMATETC fe = *lpfe;
	if (const auto hr = GetCanonicalFormatEtc(lpfe, &fe); FAILED(hr)) return hr;

	const auto cf = fe.cfFormat;

	const auto hGlobal = lpsm->hGlobal;
	
	if (const auto cbSize = ::GlobalSize(hGlobal);
		cbSize < _CalcSize( cf ))
	{
		return STG_E_MEDIUMFULL;
	}

	if (cfSakuraClipW == cf) {
		auto memory = cxx::GlobalSakura{ lpsm->hGlobal };
		memory.SetText(m_Text);
		memory.release();
	}
	else if (CF_UNICODETEXT == cf) {
		auto memory = cxx::GlobalWString{ lpsm->hGlobal };
		memory.SetText(m_Text);
		memory.release();
	}
	else if (CF_TEXT == cf) {
		auto memory = cxx::GlobalString{ lpsm->hGlobal };
		memory.SetText(CCodeFactory::ConvertToCode(m_eCodeType, m_Text).destination);
		memory.release();
	}
	else if (CF_LOCALE == cf) {
		auto memory = cxx::GlobalData<LCID>{ lpsm->hGlobal };
		memory.SetValue(MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), SORT_DEFAULT));
		memory.release();
	}
	else if (cfColumn == cf) {
		auto memory = cxx::GlobalData<BYTE>{ lpsm->hGlobal };
		memory.SetValue(0);
		memory.release();
	}
	else if (cfLine1 == cf || cfLine2 == cf) {
		auto memory = cxx::GlobalData<BYTE>{ lpsm->hGlobal };
		memory.SetValue(0x1);
		memory.release();
	}

	return S_OK;
}

/** IDataObject::QueryGetData
	@date 2008.03.26 ryoji 複数フォーマット対応
*/
STDMETHODIMP CDataObject::QueryGetData(
	LPFORMATETC lpfe
)
{
	// OLE_E_NOTRUNNING は返さない

	// 内部データは変更不可。

	if (!lpfe)
		return E_INVALIDARG;

	if (lpfe->dwAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;

	if (!(lpfe->tymed & TYMED_HGLOBAL))
		return DV_E_TYMED;

	FORMATETC fe = *lpfe;
	if (const auto hr = GetCanonicalFormatEtc(lpfe, &fe); FAILED(hr)) return hr;

	if (const auto found = std::ranges::find(m_Formats, fe.cfFormat); found == m_Formats.end())
		return DATA_E_FORMATETC;

	return S_OK;
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(
	LPFORMATETC pFormatEtcIn,
	LPFORMATETC pFormatEtcOut
)
{
	if (!pFormatEtcIn || !pFormatEtcOut)
		return E_INVALIDARG;

	if (pFormatEtcIn->lindex != -1)
		return DV_E_LINDEX;

	if (pFormatEtcIn->ptd || pFormatEtcIn->dwAspect != DVASPECT_CONTENT)
		return DATA_E_FORMATETC;

	return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP CDataObject::SetData( LPFORMATETC, LPSTGMEDIUM, BOOL )
{
	return E_NOTIMPL;
}

/** IDataObject::EnumFormatEtc
	@date 2008.03.26 ryoji IEnumFORMATETCをサポート
*/
STDMETHODIMP CDataObject::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
	if( dwDirection != DATADIR_GET )
		return S_FALSE;

	const auto pEnum = CEnumFORMATETC::make_instance(m_Formats);
	*ppenumFormatetc = pEnum;
	pEnum->AddRef();

	return *ppenumFormatetc? S_OK: S_FALSE;
}

STDMETHODIMP CDataObject::DAdvise( LPFORMATETC, DWORD, LPADVISESINK, LPDWORD )
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::DUnadvise( DWORD )
{
	return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CDataObject::EnumDAdvise( LPENUMSTATDATA* )
{
	return OLE_E_ADVISENOTSUPPORTED;
}

SIZE_T CDataObject::_CalcSize(CLIPFORMAT cf) const noexcept
{
	if (cfSakuraClipW == cf) {
		return cxx::GlobalSakura::CalcSize(m_Text);
	}
	if (CF_UNICODETEXT == cf) {
		return (std::size(m_Text) + 1) * sizeof(WCHAR);
	}
	if (CF_TEXT == cf) {
		return std::size(m_Text) + 1;
	}
	if (CF_LOCALE == cf) {
		return 4;
	}
	return 1;
}

/*!
 * @brief IEnumFORMATETC::Next
 *
 * @param[in] celt 取得する要素の数
 * @param[out] rgelt 取得した要素を格納する配列
 * @param[out, opt] pceltFetched 取得した要素数の格納先。（celtが1の場合のみ省略可。）
 * @refval S_OK      要求された数（celt）の要素をすべて正常に取得できた。
 * @refval S_FALSE   要求された数より少ない要素しか取得できなかった（列挙の末尾に達した）。
 * @refval E_POINTER rgelt が無効なポインタである（または celt > 1 なのに pceltFetched が NULL である）。
 *
 * @date 2008.03.26 ryoji 新規作成
 */
STDMETHODIMP CEnumFORMATETC::Next(
	ULONG celt,
	FORMATETC* rgelt,
	ULONG* pceltFetched
)
{
	if (0 < celt && !rgelt ||
		celt != 1 && !pceltFetched)
	{
		return E_POINTER;
	}

	if (0 == celt ||
		std::size(m_Data) <= m_nIndex)
	{
		return S_FALSE;
	}

	ULONG i = celt;
	while( m_nIndex < std::size(m_Data) && i > 0 ){
		(*rgelt).cfFormat = static_cast<CLIPFORMAT>(m_Data[m_nIndex]);
		(*rgelt).ptd = nullptr;
		(*rgelt).dwAspect = DVASPECT_CONTENT;
		(*rgelt).lindex = -1;
		(*rgelt).tymed = TYMED_HGLOBAL;
		rgelt++;
		m_nIndex++;
		i--;
	}
	if( pceltFetched != nullptr )
		*pceltFetched = celt - i;

	return (i == 0)? S_OK : S_FALSE;
}

/** IEnumFORMATETC::Skip
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP CEnumFORMATETC::Skip(ULONG celt)
{
	while (m_nIndex < std::size(m_Data) && celt > 0) {
		++m_nIndex;
		--celt;
	}

	return (celt == 0)? S_OK : S_FALSE;
}

/** IEnumFORMATETC::Reset
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP CEnumFORMATETC::Reset(void)
{
	m_nIndex = 0;
	return S_OK;
}

/** IEnumFORMATETC::Clone
	@date 2008.03.26 ryoji 新規作成
*/
STDMETHODIMP CEnumFORMATETC::Clone([[maybe_unused]] IEnumFORMATETC** ppenum)
{
	return E_NOTIMPL;
}
