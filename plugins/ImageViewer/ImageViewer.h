

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Fri Sep 10 22:41:57 2004
 */
/* Compiler settings for ImageViewer.idl:
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


#ifndef __ImageViewer_h__
#define __ImageViewer_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ImageViewerPlugin_FWD_DEFINED__
#define __ImageViewerPlugin_FWD_DEFINED__

#ifdef __cplusplus
typedef class ImageViewerPlugin ImageViewerPlugin;
#else
typedef struct ImageViewerPlugin ImageViewerPlugin;
#endif /* __cplusplus */

#endif 	/* __ImageViewerPlugin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_ImageViewer_0000 */
/* [local] */ 

#include "ShareazaOM.h"


extern RPC_IF_HANDLE __MIDL_itf_ImageViewer_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ImageViewer_0000_v0_0_s_ifspec;


#ifndef __ImageViewerLib_LIBRARY_DEFINED__
#define __ImageViewerLib_LIBRARY_DEFINED__

/* library ImageViewerLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ImageViewerLib;

EXTERN_C const CLSID CLSID_ImageViewerPlugin;

#ifdef __cplusplus

class DECLSPEC_UUID("2EE9D739-7726-41cf-8F18-4B1B8763BC63")
ImageViewerPlugin;
#endif
#endif /* __ImageViewerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


