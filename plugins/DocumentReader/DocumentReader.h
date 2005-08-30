

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Tue Aug 30 10:06:37 2005
 */
/* Compiler settings for .\DocumentReader.idl:
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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __DocumentReader_h__
#define __DocumentReader_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IDocReader_FWD_DEFINED__
#define __IDocReader_FWD_DEFINED__
typedef interface IDocReader IDocReader;
#endif 	/* __IDocReader_FWD_DEFINED__ */


#ifndef __DocReader_FWD_DEFINED__
#define __DocReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class DocReader DocReader;
#else
typedef struct DocReader DocReader;
#endif /* __cplusplus */

#endif 	/* __DocReader_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_DocumentReader_0000 */
/* [local] */ 

#include <ShareazaOM.h>


extern RPC_IF_HANDLE __MIDL_itf_DocumentReader_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_DocumentReader_0000_v0_0_s_ifspec;

#ifndef __IDocReader_INTERFACE_DEFINED__
#define __IDocReader_INTERFACE_DEFINED__

/* interface IDocReader */
/* [hidden][hidden][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDocReader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2285F261-F049-4944-A4AA-C93168D01231")
    IDocReader : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IDocReaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDocReader * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDocReader * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDocReader * This);
        
        END_INTERFACE
    } IDocReaderVtbl;

    interface IDocReader
    {
        CONST_VTBL struct IDocReaderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDocReader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDocReader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDocReader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDocReader_INTERFACE_DEFINED__ */



#ifndef __DocumentReaderLib_LIBRARY_DEFINED__
#define __DocumentReaderLib_LIBRARY_DEFINED__

/* library DocumentReaderLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DocumentReaderLib;

EXTERN_C const CLSID CLSID_DocReader;

#ifdef __cplusplus

class DECLSPEC_UUID("E9F51B1E-DB0F-4EEE-9B36-46151994C715")
DocReader;
#endif
#endif /* __DocumentReaderLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


