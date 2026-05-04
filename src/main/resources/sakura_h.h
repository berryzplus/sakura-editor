

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 12:14:07 2038
 */
/* Compiler settings for ..\src\main\resources\sakura.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __sakura_h_h__
#define __sakura_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __ITrayWnd_FWD_DEFINED__
#define __ITrayWnd_FWD_DEFINED__
typedef interface ITrayWnd ITrayWnd;

#endif 	/* __ITrayWnd_FWD_DEFINED__ */


#ifndef __TrayWnd_FWD_DEFINED__
#define __TrayWnd_FWD_DEFINED__

#ifdef __cplusplus
typedef class TrayWnd TrayWnd;
#else
typedef struct TrayWnd TrayWnd;
#endif /* __cplusplus */

#endif 	/* __TrayWnd_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ITrayWnd_INTERFACE_DEFINED__
#define __ITrayWnd_INTERFACE_DEFINED__

/* interface ITrayWnd */
/* [unique][oleautomation][dual][uuid][object] */ 


EXTERN_C const IID IID_ITrayWnd;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BB2F50E1-4785-47FD-8728-E750A6EC96D2")
    ITrayWnd : public IDispatch
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ShowPopupL( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ShowPopupR( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OpenNewEditor( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ITrayWndVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITrayWnd * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITrayWnd * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITrayWnd * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITrayWnd * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITrayWnd * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITrayWnd * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITrayWnd * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(ITrayWnd, ShowPopupL)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ShowPopupL )( 
            ITrayWnd * This);
        
        DECLSPEC_XFGVIRT(ITrayWnd, ShowPopupR)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ShowPopupR )( 
            ITrayWnd * This);
        
        DECLSPEC_XFGVIRT(ITrayWnd, OpenNewEditor)
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OpenNewEditor )( 
            ITrayWnd * This);
        
        END_INTERFACE
    } ITrayWndVtbl;

    interface ITrayWnd
    {
        CONST_VTBL struct ITrayWndVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITrayWnd_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITrayWnd_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITrayWnd_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITrayWnd_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITrayWnd_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITrayWnd_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITrayWnd_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITrayWnd_ShowPopupL(This)	\
    ( (This)->lpVtbl -> ShowPopupL(This) ) 

#define ITrayWnd_ShowPopupR(This)	\
    ( (This)->lpVtbl -> ShowPopupR(This) ) 

#define ITrayWnd_OpenNewEditor(This)	\
    ( (This)->lpVtbl -> OpenNewEditor(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ITrayWnd_INTERFACE_DEFINED__ */



#ifndef __SakuraEditorLib_LIBRARY_DEFINED__
#define __SakuraEditorLib_LIBRARY_DEFINED__

/* library SakuraEditorLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_SakuraEditorLib;

EXTERN_C const CLSID CLSID_TrayWnd;

#ifdef __cplusplus

class DECLSPEC_UUID("4113FEFC-018E-4B77-811D-AD328E0B38EE")
TrayWnd;
#endif
#endif /* __SakuraEditorLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


