

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Tue Nov 01 15:07:27 2005
 */
/* Compiler settings for .\MediaImageServices.idl:
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


#ifndef __MediaImageServices_h__
#define __MediaImageServices_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __VideoReader_FWD_DEFINED__
#define __VideoReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class VideoReader VideoReader;
#else
typedef struct VideoReader VideoReader;
#endif /* __cplusplus */

#endif 	/* __VideoReader_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __MediaImageServicesLib_LIBRARY_DEFINED__
#define __MediaImageServicesLib_LIBRARY_DEFINED__

/* library MediaImageServicesLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_MediaImageServicesLib;

EXTERN_C const CLSID CLSID_VideoReader;

#ifdef __cplusplus

class DECLSPEC_UUID("04CC76C7-1ED7-4CAE-9762-B8664ED008ED")
VideoReader;
#endif
#endif /* __MediaImageServicesLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


