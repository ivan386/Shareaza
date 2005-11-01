

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Tue Nov 01 15:03:56 2005
 */
/* Compiler settings for .\SWFPlugin.idl:
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


#ifndef __SWFPlugin_h__
#define __SWFPlugin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __SWFReader_FWD_DEFINED__
#define __SWFReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWFReader SWFReader;
#else
typedef struct SWFReader SWFReader;
#endif /* __cplusplus */

#endif 	/* __SWFReader_FWD_DEFINED__ */


#ifndef __SWFBuilder_FWD_DEFINED__
#define __SWFBuilder_FWD_DEFINED__

#ifdef __cplusplus
typedef class SWFBuilder SWFBuilder;
#else
typedef struct SWFBuilder SWFBuilder;
#endif /* __cplusplus */

#endif 	/* __SWFBuilder_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __SWFPluginLib_LIBRARY_DEFINED__
#define __SWFPluginLib_LIBRARY_DEFINED__

/* library SWFPluginLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_SWFPluginLib;

EXTERN_C const CLSID CLSID_SWFReader;

#ifdef __cplusplus

class DECLSPEC_UUID("FC4D8F69-0B18-49BB-8AB7-87EB77AA1A9D")
SWFReader;
#endif

EXTERN_C const CLSID CLSID_SWFBuilder;

#ifdef __cplusplus

class DECLSPEC_UUID("B978F591-5137-4612-873A-DC2081BAD6CD")
SWFBuilder;
#endif
#endif /* __SWFPluginLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


