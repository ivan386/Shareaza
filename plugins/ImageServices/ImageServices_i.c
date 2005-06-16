

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Wed Jun 15 18:22:25 2005
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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_ImageServicesLib,0xBFB299FD,0x5773,0x478B,0x80,0x33,0x19,0x77,0x09,0x3F,0x1C,0x4C);


MIDL_DEFINE_GUID(CLSID, CLSID_JPEGReader,0x5E6309F2,0x9971,0x4683,0x94,0x45,0xF5,0x48,0xE8,0x1B,0xEC,0x07);


MIDL_DEFINE_GUID(CLSID, CLSID_PNGReader,0xD427C22F,0x23FB,0x4E51,0xA8,0xB8,0x70,0xF2,0x03,0x6E,0xD3,0xBA);


MIDL_DEFINE_GUID(CLSID, CLSID_GIFReader,0xDD99484A,0x33B7,0x46C2,0x83,0x87,0x2F,0xBE,0xD7,0xA2,0x5A,0x54);


MIDL_DEFINE_GUID(CLSID, CLSID_AVIThumb,0x4956C5F5,0xD9A8,0x4CBB,0x89,0x94,0xF5,0x3C,0xF5,0x5C,0xFD,0xF5);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



