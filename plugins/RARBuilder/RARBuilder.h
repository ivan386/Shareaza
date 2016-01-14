

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Thu Sep 24 21:55:36 2015
 */
/* Compiler settings for .\RARBuilder.idl:
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


#ifndef __RARBuilder_h__
#define __RARBuilder_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __RARBuilder_FWD_DEFINED__
#define __RARBuilder_FWD_DEFINED__

#ifdef __cplusplus
typedef class RARBuilder RARBuilder;
#else
typedef struct RARBuilder RARBuilder;
#endif /* __cplusplus */

#endif 	/* __RARBuilder_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __RARBuilderLib_LIBRARY_DEFINED__
#define __RARBuilderLib_LIBRARY_DEFINED__

/* library RARBuilderLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_RARBuilderLib;

EXTERN_C const CLSID CLSID_RARBuilder;

#ifdef __cplusplus

class DECLSPEC_UUID("F801DAD7-F08D-48EF-B0DF-6B120377E835")
RARBuilder;
#endif
#endif /* __RARBuilderLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


