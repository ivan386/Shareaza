
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0347 */
/* at Thu Nov 08 17:04:26 2007
 */
/* Compiler settings for .\SkinScanSKS.idl:
    Os, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __SkinScanSKS_h__
#define __SkinScanSKS_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __SkinScanSKS_FWD_DEFINED__
#define __SkinScanSKS_FWD_DEFINED__

#ifdef __cplusplus
typedef class SkinScanSKS SkinScanSKS;
#else
typedef struct SkinScanSKS SkinScanSKS;
#endif /* __cplusplus */

#endif 	/* __SkinScanSKS_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __SkinScanSKSLib_LIBRARY_DEFINED__
#define __SkinScanSKSLib_LIBRARY_DEFINED__

/* library SkinScanSKSLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_SkinScanSKSLib;

EXTERN_C const CLSID CLSID_SkinScanSKS;

#ifdef __cplusplus

class DECLSPEC_UUID("A4F1E383-B493-4580-8DB6-5CC89CBAAC53")
SkinScanSKS;
#endif
#endif /* __SkinScanSKSLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


