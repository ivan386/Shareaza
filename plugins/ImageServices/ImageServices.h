

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Mon Oct 11 16:05:57 2004
 */
/* Compiler settings for .\ImageServices.idl:
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


#ifndef __ImageServices_h__
#define __ImageServices_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __JPEGReader_FWD_DEFINED__
#define __JPEGReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class JPEGReader JPEGReader;
#else
typedef struct JPEGReader JPEGReader;
#endif /* __cplusplus */

#endif 	/* __JPEGReader_FWD_DEFINED__ */


#ifndef __PNGReader_FWD_DEFINED__
#define __PNGReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class PNGReader PNGReader;
#else
typedef struct PNGReader PNGReader;
#endif /* __cplusplus */

#endif 	/* __PNGReader_FWD_DEFINED__ */


#ifndef __GIFReader_FWD_DEFINED__
#define __GIFReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class GIFReader GIFReader;
#else
typedef struct GIFReader GIFReader;
#endif /* __cplusplus */

#endif 	/* __GIFReader_FWD_DEFINED__ */


#ifndef __AVIThumb_FWD_DEFINED__
#define __AVIThumb_FWD_DEFINED__

#ifdef __cplusplus
typedef class AVIThumb AVIThumb;
#else
typedef struct AVIThumb AVIThumb;
#endif /* __cplusplus */

#endif 	/* __AVIThumb_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_ImageServices_0000 */
/* [local] */ 

#include <ShareazaOM.h>


extern RPC_IF_HANDLE __MIDL_itf_ImageServices_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ImageServices_0000_v0_0_s_ifspec;


#ifndef __ImageServicesLib_LIBRARY_DEFINED__
#define __ImageServicesLib_LIBRARY_DEFINED__

/* library ImageServicesLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ImageServicesLib;

EXTERN_C const CLSID CLSID_JPEGReader;

#ifdef __cplusplus

class DECLSPEC_UUID("5E6309F2-9971-4683-9445-F548E81BEC07")
JPEGReader;
#endif

EXTERN_C const CLSID CLSID_PNGReader;

#ifdef __cplusplus

class DECLSPEC_UUID("D427C22F-23FB-4E51-A8B8-70F2036ED3BA")
PNGReader;
#endif

EXTERN_C const CLSID CLSID_GIFReader;

#ifdef __cplusplus

class DECLSPEC_UUID("DD99484A-33B7-46C2-8387-2FBED7A25A54")
GIFReader;
#endif

EXTERN_C const CLSID CLSID_AVIThumb;

#ifdef __cplusplus

class DECLSPEC_UUID("4956C5F5-D9A8-4CBB-8994-F53CF55CFDF5")
AVIThumb;
#endif
#endif /* __ImageServicesLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


