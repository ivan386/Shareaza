

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Thu Sep 24 21:56:44 2015
 */
/* Compiler settings for .\RatDVDReader.idl:
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


#ifndef __RatDVDReader_h__
#define __RatDVDReader_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IImageServicePlugin_FWD_DEFINED__
#define __IImageServicePlugin_FWD_DEFINED__
typedef interface IImageServicePlugin IImageServicePlugin;
#endif 	/* __IImageServicePlugin_FWD_DEFINED__ */


#ifndef __ILibraryBuilderPlugin_FWD_DEFINED__
#define __ILibraryBuilderPlugin_FWD_DEFINED__
typedef interface ILibraryBuilderPlugin ILibraryBuilderPlugin;
#endif 	/* __ILibraryBuilderPlugin_FWD_DEFINED__ */


#ifndef __RatDVDReader_FWD_DEFINED__
#define __RatDVDReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class RatDVDReader RatDVDReader;
#else
typedef struct RatDVDReader RatDVDReader;
#endif /* __cplusplus */

#endif 	/* __RatDVDReader_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __RatDVDReaderLib_LIBRARY_DEFINED__
#define __RatDVDReaderLib_LIBRARY_DEFINED__

/* library RatDVDReaderLib */
/* [helpstring][version][uuid] */ 




EXTERN_C const IID LIBID_RatDVDReaderLib;

#ifndef __IImageServicePlugin_INTERFACE_DEFINED__
#define __IImageServicePlugin_INTERFACE_DEFINED__

/* interface IImageServicePlugin */
/* [object][oleautomation][uuid] */ 


EXTERN_C const IID IID_IImageServicePlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AB0A7BF2-94C0-4daa-8256-2BB6C3648050")
    IImageServicePlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LoadFromFile( 
            /* [in] */ BSTR sFile,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY * *ppImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadFromMemory( 
            /* [in] */ BSTR sType,
            /* [in] */ SAFEARRAY * pMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY * *ppImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveToFile( 
            /* [in] */ BSTR sFile,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY * pImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveToMemory( 
            /* [in] */ BSTR sType,
            /* [out] */ SAFEARRAY * *ppMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY * pImage) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImageServicePluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImageServicePlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImageServicePlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImageServicePlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *LoadFromFile )( 
            IImageServicePlugin * This,
            /* [in] */ BSTR sFile,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY * *ppImage);
        
        HRESULT ( STDMETHODCALLTYPE *LoadFromMemory )( 
            IImageServicePlugin * This,
            /* [in] */ BSTR sType,
            /* [in] */ SAFEARRAY * pMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY * *ppImage);
        
        HRESULT ( STDMETHODCALLTYPE *SaveToFile )( 
            IImageServicePlugin * This,
            /* [in] */ BSTR sFile,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY * pImage);
        
        HRESULT ( STDMETHODCALLTYPE *SaveToMemory )( 
            IImageServicePlugin * This,
            /* [in] */ BSTR sType,
            /* [out] */ SAFEARRAY * *ppMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY * pImage);
        
        END_INTERFACE
    } IImageServicePluginVtbl;

    interface IImageServicePlugin
    {
        CONST_VTBL struct IImageServicePluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImageServicePlugin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IImageServicePlugin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IImageServicePlugin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IImageServicePlugin_LoadFromFile(This,sFile,pParams,ppImage)	\
    ( (This)->lpVtbl -> LoadFromFile(This,sFile,pParams,ppImage) ) 

#define IImageServicePlugin_LoadFromMemory(This,sType,pMemory,pParams,ppImage)	\
    ( (This)->lpVtbl -> LoadFromMemory(This,sType,pMemory,pParams,ppImage) ) 

#define IImageServicePlugin_SaveToFile(This,sFile,pParams,pImage)	\
    ( (This)->lpVtbl -> SaveToFile(This,sFile,pParams,pImage) ) 

#define IImageServicePlugin_SaveToMemory(This,sType,ppMemory,pParams,pImage)	\
    ( (This)->lpVtbl -> SaveToMemory(This,sType,ppMemory,pParams,pImage) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IImageServicePlugin_LoadFromFile_Proxy( 
    IImageServicePlugin * This,
    /* [in] */ BSTR sFile,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [out] */ SAFEARRAY * *ppImage);


void __RPC_STUB IImageServicePlugin_LoadFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImageServicePlugin_LoadFromMemory_Proxy( 
    IImageServicePlugin * This,
    /* [in] */ BSTR sType,
    /* [in] */ SAFEARRAY * pMemory,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [out] */ SAFEARRAY * *ppImage);


void __RPC_STUB IImageServicePlugin_LoadFromMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImageServicePlugin_SaveToFile_Proxy( 
    IImageServicePlugin * This,
    /* [in] */ BSTR sFile,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [in] */ SAFEARRAY * pImage);


void __RPC_STUB IImageServicePlugin_SaveToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImageServicePlugin_SaveToMemory_Proxy( 
    IImageServicePlugin * This,
    /* [in] */ BSTR sType,
    /* [out] */ SAFEARRAY * *ppMemory,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [in] */ SAFEARRAY * pImage);


void __RPC_STUB IImageServicePlugin_SaveToMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IImageServicePlugin_INTERFACE_DEFINED__ */


#ifndef __ILibraryBuilderPlugin_INTERFACE_DEFINED__
#define __ILibraryBuilderPlugin_INTERFACE_DEFINED__

/* interface ILibraryBuilderPlugin */
/* [oleautomation][uuid][object] */ 


EXTERN_C const IID IID_ILibraryBuilderPlugin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("32496CEA-3B51-4f2f-9CE7-73D6AC942C34")
    ILibraryBuilderPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Process( 
            /* [in] */ BSTR sFile,
            /* [in] */ ISXMLElement *pXML) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILibraryBuilderPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILibraryBuilderPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILibraryBuilderPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILibraryBuilderPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *Process )( 
            ILibraryBuilderPlugin * This,
            /* [in] */ BSTR sFile,
            /* [in] */ ISXMLElement *pXML);
        
        END_INTERFACE
    } ILibraryBuilderPluginVtbl;

    interface ILibraryBuilderPlugin
    {
        CONST_VTBL struct ILibraryBuilderPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILibraryBuilderPlugin_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ILibraryBuilderPlugin_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ILibraryBuilderPlugin_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ILibraryBuilderPlugin_Process(This,sFile,pXML)	\
    ( (This)->lpVtbl -> Process(This,sFile,pXML) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ILibraryBuilderPlugin_Process_Proxy( 
    ILibraryBuilderPlugin * This,
    /* [in] */ BSTR sFile,
    /* [in] */ ISXMLElement *pXML);


void __RPC_STUB ILibraryBuilderPlugin_Process_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILibraryBuilderPlugin_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_RatDVDReader;

#ifdef __cplusplus

class DECLSPEC_UUID("61700EEC-D5D3-4793-BD1F-514896D67F44")
RatDVDReader;
#endif
#endif /* __RatDVDReaderLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


