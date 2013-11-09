

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Fri Nov 01 16:55:33 2013
 */
/* Compiler settings for .\VirusTotal.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __VirusTotal_h__
#define __VirusTotal_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGeneralPlugin_FWD_DEFINED__
#define __IGeneralPlugin_FWD_DEFINED__
typedef interface IGeneralPlugin IGeneralPlugin;
#endif 	/* __IGeneralPlugin_FWD_DEFINED__ */


#ifndef __ICommandPlugin_FWD_DEFINED__
#define __ICommandPlugin_FWD_DEFINED__
typedef interface ICommandPlugin ICommandPlugin;
#endif 	/* __ICommandPlugin_FWD_DEFINED__ */


#ifndef __Plugin_FWD_DEFINED__
#define __Plugin_FWD_DEFINED__

#ifdef __cplusplus
typedef class Plugin Plugin;
#else
typedef struct Plugin Plugin;
#endif /* __cplusplus */

#endif 	/* __Plugin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __VirusTotalLib_LIBRARY_DEFINED__
#define __VirusTotalLib_LIBRARY_DEFINED__

/* library VirusTotalLib */
/* [helpstring][version][uuid] */ 




EXTERN_C const IID LIBID_VirusTotalLib;

#ifndef __IGeneralPlugin_INTERFACE_DEFINED__
#define __IGeneralPlugin_INTERFACE_DEFINED__

/* interface IGeneralPlugin */
/* [object][oleautomation][uuid] */ 


EXTERN_C const IID IID_IGeneralPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D1B5D3A4-B890-470a-A3FF-9700F3C2A063")
    IGeneralPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetApplication( 
            /* [in] */ IApplication *pApplication) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCapabilities( 
            /* [out] */ DWORD *pnCaps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSkinChanged( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGeneralPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGeneralPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGeneralPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGeneralPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetApplication )( 
            IGeneralPlugin * This,
            /* [in] */ IApplication *pApplication);
        
        HRESULT ( STDMETHODCALLTYPE *QueryCapabilities )( 
            IGeneralPlugin * This,
            /* [out] */ DWORD *pnCaps);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            IGeneralPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnSkinChanged )( 
            IGeneralPlugin * This);
        
        END_INTERFACE
    } IGeneralPluginVtbl;

    interface IGeneralPlugin
    {
        CONST_VTBL struct IGeneralPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGeneralPlugin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGeneralPlugin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGeneralPlugin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGeneralPlugin_SetApplication(This,pApplication)	\
    ( (This)->lpVtbl -> SetApplication(This,pApplication) ) 

#define IGeneralPlugin_QueryCapabilities(This,pnCaps)	\
    ( (This)->lpVtbl -> QueryCapabilities(This,pnCaps) ) 

#define IGeneralPlugin_Configure(This)	\
    ( (This)->lpVtbl -> Configure(This) ) 

#define IGeneralPlugin_OnSkinChanged(This)	\
    ( (This)->lpVtbl -> OnSkinChanged(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IGeneralPlugin_SetApplication_Proxy( 
    IGeneralPlugin * This,
    /* [in] */ IApplication *pApplication);


void __RPC_STUB IGeneralPlugin_SetApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGeneralPlugin_QueryCapabilities_Proxy( 
    IGeneralPlugin * This,
    /* [out] */ DWORD *pnCaps);


void __RPC_STUB IGeneralPlugin_QueryCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGeneralPlugin_Configure_Proxy( 
    IGeneralPlugin * This);


void __RPC_STUB IGeneralPlugin_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGeneralPlugin_OnSkinChanged_Proxy( 
    IGeneralPlugin * This);


void __RPC_STUB IGeneralPlugin_OnSkinChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGeneralPlugin_INTERFACE_DEFINED__ */


#ifndef __ICommandPlugin_INTERFACE_DEFINED__
#define __ICommandPlugin_INTERFACE_DEFINED__

/* interface ICommandPlugin */
/* [object][oleautomation][uuid] */ 


EXTERN_C const IID IID_ICommandPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CB25DAED-D745-45db-994E-32639D2888A9")
    ICommandPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterCommands( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertCommands( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUpdate( 
            /* [in] */ UINT nCommandID,
            /* [out][in] */ TRISTATE *pbVisible,
            /* [out][in] */ TRISTATE *pbEnabled,
            /* [out][in] */ TRISTATE *pbChecked) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCommand( 
            /* [in] */ UINT nCommandID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterCommands )( 
            ICommandPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *InsertCommands )( 
            ICommandPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnUpdate )( 
            ICommandPlugin * This,
            /* [in] */ UINT nCommandID,
            /* [out][in] */ TRISTATE *pbVisible,
            /* [out][in] */ TRISTATE *pbEnabled,
            /* [out][in] */ TRISTATE *pbChecked);
        
        HRESULT ( STDMETHODCALLTYPE *OnCommand )( 
            ICommandPlugin * This,
            /* [in] */ UINT nCommandID);
        
        END_INTERFACE
    } ICommandPluginVtbl;

    interface ICommandPlugin
    {
        CONST_VTBL struct ICommandPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandPlugin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICommandPlugin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICommandPlugin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICommandPlugin_RegisterCommands(This)	\
    ( (This)->lpVtbl -> RegisterCommands(This) ) 

#define ICommandPlugin_InsertCommands(This)	\
    ( (This)->lpVtbl -> InsertCommands(This) ) 

#define ICommandPlugin_OnUpdate(This,nCommandID,pbVisible,pbEnabled,pbChecked)	\
    ( (This)->lpVtbl -> OnUpdate(This,nCommandID,pbVisible,pbEnabled,pbChecked) ) 

#define ICommandPlugin_OnCommand(This,nCommandID)	\
    ( (This)->lpVtbl -> OnCommand(This,nCommandID) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandPlugin_RegisterCommands_Proxy( 
    ICommandPlugin * This);


void __RPC_STUB ICommandPlugin_RegisterCommands_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPlugin_InsertCommands_Proxy( 
    ICommandPlugin * This);


void __RPC_STUB ICommandPlugin_InsertCommands_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPlugin_OnUpdate_Proxy( 
    ICommandPlugin * This,
    /* [in] */ UINT nCommandID,
    /* [out][in] */ TRISTATE *pbVisible,
    /* [out][in] */ TRISTATE *pbEnabled,
    /* [out][in] */ TRISTATE *pbChecked);


void __RPC_STUB ICommandPlugin_OnUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPlugin_OnCommand_Proxy( 
    ICommandPlugin * This,
    /* [in] */ UINT nCommandID);


void __RPC_STUB ICommandPlugin_OnCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandPlugin_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_Plugin;

#ifdef __cplusplus

class DECLSPEC_UUID("76F13243-9F62-4241-AC07-3B359BBE4EC5")
Plugin;
#endif
#endif /* __VirusTotalLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


