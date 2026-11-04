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
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDROPTARGET_365C215D_E844_49BD_8E96_549970AE0A50_H_
#define SAKURA_CDROPTARGET_365C215D_E844_49BD_8E96_549970AE0A50_H_
#pragma once

#include <Unknwn.h>

#include "charset/CCodeFactory.h"
#include "cxx/TComImpl.hpp"
#include "util/design_template.h"
#include "util/os.h"

class CDropTarget;
class CYbInterfaceBase;
class CEditWnd;	// 2008.06.20 ryoji
class CEditView;// 2002/2/3 aroka ヘッダー軽量化

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class COleLibrary
{
	using Me = COleLibrary;

	friend class CYbInterfaceBase;
private:
//	DWORD m_dwCount;	// 2009.01.08 ryoji m_dwCount削除
	COleLibrary();
public:
	COleLibrary(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	COleLibrary(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~COleLibrary();
private:
	void Initialize();
	void UnInitialize();
};

class CYbInterfaceBase
{
	using Me = CYbInterfaceBase;

	static COleLibrary m_olelib;
protected:
	CYbInterfaceBase();
	CYbInterfaceBase(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CYbInterfaceBase(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CYbInterfaceBase();
	static HRESULT QueryInterfaceImpl( IUnknown*, REFIID, REFIID, void** );
};

template<class BASEINTERFACE>
class CYbInterfaceImpl : public BASEINTERFACE, public CYbInterfaceBase
{
private:
	static REFIID m_owniid;
public:
	CYbInterfaceImpl(){AddRef();}
	STDMETHOD( QueryInterface )( REFIID riid, void** ppvObj )
	{return QueryInterfaceImpl( this, m_owniid, riid, ppvObj );}
	STDMETHOD_( ULONG, AddRef )( void )
	{return 1;}
	STDMETHOD_( ULONG, Release )( void )
	{return 0;}
};

class CDropTarget : public cxx::TComImpl<IDropTarget>
{
private:
	using Base = cxx::TComImpl<IDropTarget>;
	using Me = CDropTarget;

	/*
	||  Constructors
	*/
protected:
	CDropTarget() = default;

	/*
	||  Attributes & Operations
	*/

	/*
	||  実装ヘルパ関数
	*/
};

class CDropTarget_CEditWnd : public CDropTarget
{
private:
	using Base = CDropTarget;
	using Me = CDropTarget_CEditWnd;

public:
	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<Me, Args...>
	{
		return Base::template make_instance<Me>(std::forward<Args>(args)...);
	}

	CDropTarget_CEditWnd() = default;

	IFACEMETHODIMP	DragEnter(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragLeave() override;
	IFACEMETHODIMP	Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
};

class CDropTarget_CEditView : public CDropTarget
{
private:
	using Base = CDropTarget;
	using Me = CDropTarget_CEditView;

public:
	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<Me, Args...>
	{
		return Base::template make_instance<Me>(std::forward<Args>(args)...);
	}

	explicit CDropTarget_CEditView(CEditView* pcEditView)
		: m_pcEditView(pcEditView)
	{
	}

	IFACEMETHODIMP	DragEnter(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;
	IFACEMETHODIMP	DragLeave() override;
	IFACEMETHODIMP	Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect) override;

private:
	CEditView* m_pcEditView = nullptr;
};

class CDropSource final : public cxx::TComImpl<IDropSource> {
private:
	using Base = cxx::TComImpl<IDropSource>;
	using Me = CDropSource;

public:
	explicit CDropSource(bool bLeft) : m_bLeft(bLeft) {}

	STDMETHOD( QueryContinueDrag )( BOOL bEscapePressed, DWORD dwKeyState );
	STDMETHOD( GiveFeedback )( DWORD dropEffect );

	DWORD	DoDragDrop(LPDATAOBJECT pDataObject, DWORD dwDesiredEffects);

private:
	bool m_bLeft;
};

class CDataObject final : public cxx::TComImpl<IDataObject> {
private:
	using Base = cxx::TComImpl<IDataObject>;
	using Me = CDataObject;

	std::wstring m_Text;
	ECodeType m_eCodeType;

	std::vector<UINT> m_Formats{ CF_UNICODETEXT };

public:
	static UINT cfSakuraClipW;
	static UINT cfColumn;
	static UINT cfLine1;
	static UINT cfLine2;

	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<Me, Args...>
	{
		return Base::template make_instance<Me>(std::forward<Args>(args)...);
	}

	explicit CDataObject(
		std::wstring_view text,
		bool bColumnSelect = false,
		bool bLineSelect = false,
		ECodeType eCharset = ECodeType::CODE_NONE
	);

	STDMETHOD( GetData )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( GetDataHere )( LPFORMATETC, LPSTGMEDIUM );
	STDMETHOD( QueryGetData )( LPFORMATETC );
	STDMETHOD( GetCanonicalFormatEtc )( LPFORMATETC, LPFORMATETC );
	STDMETHOD( SetData )( LPFORMATETC, LPSTGMEDIUM, BOOL );
	STDMETHOD( EnumFormatEtc )( DWORD, IEnumFORMATETC** );
	STDMETHOD( DAdvise )( LPFORMATETC, DWORD, LPADVISESINK, LPDWORD );
	STDMETHOD( DUnadvise )( DWORD );
	STDMETHOD( EnumDAdvise )( LPENUMSTATDATA* );

private:
	SIZE_T	_CalcSize(CLIPFORMAT cf) const noexcept;
};

//! CEnumFORMATETC クラス
//	2008.03.26 ryoji 新規作成
class CEnumFORMATETC final : public cxx::TComImpl<IEnumFORMATETC> {
private:
	using Base = cxx::TComImpl<IEnumFORMATETC>;
	using Me = CEnumFORMATETC;

	std::vector<UINT> m_Data;
	size_t m_nIndex = 0;

public:
	// 生成関数
	template<typename... Args>
	static com_pointer_type make_instance(Args&&... args)
		requires std::constructible_from<Me, Args...>
	{
		return Base::template make_instance<Me>(std::forward<Args>(args)...);
	}

	explicit CEnumFORMATETC(
		std::span<UINT> data
	)
		: m_Data(data.begin(), data.end())
	{
	}

	STDMETHOD( Next )(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
	STDMETHOD( Skip )(ULONG celt);
	STDMETHOD( Reset )(void);
	STDMETHOD( Clone )(IEnumFORMATETC** ppenum);
};

#endif /* SAKURA_CDROPTARGET_365C215D_E844_49BD_8E96_549970AE0A50_H_ */
