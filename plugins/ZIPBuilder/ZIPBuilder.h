

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Tue Oct 29 22:06:27 2013
 */
/* Compiler settings for .\ZIPBuilder.idl:
    Oicf, W1, Zp8, env=Win64 (32b run)
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


#ifndef __ZIPBuilder_h__
#define __ZIPBuilder_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ZIPBuilder_FWD_DEFINED__
#define __ZIPBuilder_FWD_DEFINED__

#ifdef __cplusplus
typedef class ZIPBuilder ZIPBuilder;
#else
typedef struct ZIPBuilder ZIPBuilder;
#endif /* __cplusplus */

#endif 	/* __ZIPBuilder_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __ZIPBuilderLib_LIBRARY_DEFINED__
#define __ZIPBuilderLib_LIBRARY_DEFINED__

/* library ZIPBuilderLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ZIPBuilderLib;

EXTERN_C const CLSID CLSID_ZIPBuilder;

#ifdef __cplusplus

class DECLSPEC_UUID("2F74AA28-2498-4805-911A-04C39858D529")
ZIPBuilder;
#endif
#endif /* __ZIPBuilderLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


