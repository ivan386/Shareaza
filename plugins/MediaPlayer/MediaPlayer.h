

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Tue Oct 29 22:05:00 2013
 */
/* Compiler settings for .\MediaPlayer.idl:
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


#ifndef __MediaPlayer_h__
#define __MediaPlayer_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __MediaPlayer_FWD_DEFINED__
#define __MediaPlayer_FWD_DEFINED__

#ifdef __cplusplus
typedef class MediaPlayer MediaPlayer;
#else
typedef struct MediaPlayer MediaPlayer;
#endif /* __cplusplus */

#endif 	/* __MediaPlayer_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "strmif.h"
#include "control.h"
#include "Shareaza.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __MediaPlayerLib_LIBRARY_DEFINED__
#define __MediaPlayerLib_LIBRARY_DEFINED__

/* library MediaPlayerLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_MediaPlayerLib;

EXTERN_C const CLSID CLSID_MediaPlayer;

#ifdef __cplusplus

class DECLSPEC_UUID("3DC28AA6-A597-4E03-96DF-ADA19155B0BE")
MediaPlayer;
#endif
#endif /* __MediaPlayerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


