

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Wed Jul 16 23:01:56 2014
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


#ifndef __DocumentReader_h__
#define __DocumentReader_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

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
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 



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


