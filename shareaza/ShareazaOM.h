

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Tue Jun 01 13:11:58 2004
 */
/* Compiler settings for .\Shareaza.idl:
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


#ifndef __ShareazaOM_h__
#define __ShareazaOM_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IApplication_FWD_DEFINED__
#define __IApplication_FWD_DEFINED__
typedef interface IApplication IApplication;
#endif 	/* __IApplication_FWD_DEFINED__ */


#ifndef __ISXMLNode_FWD_DEFINED__
#define __ISXMLNode_FWD_DEFINED__
typedef interface ISXMLNode ISXMLNode;
#endif 	/* __ISXMLNode_FWD_DEFINED__ */


#ifndef __ISXMLElement_FWD_DEFINED__
#define __ISXMLElement_FWD_DEFINED__
typedef interface ISXMLElement ISXMLElement;
#endif 	/* __ISXMLElement_FWD_DEFINED__ */


#ifndef __ISXMLElements_FWD_DEFINED__
#define __ISXMLElements_FWD_DEFINED__
typedef interface ISXMLElements ISXMLElements;
#endif 	/* __ISXMLElements_FWD_DEFINED__ */


#ifndef __ISXMLAttribute_FWD_DEFINED__
#define __ISXMLAttribute_FWD_DEFINED__
typedef interface ISXMLAttribute ISXMLAttribute;
#endif 	/* __ISXMLAttribute_FWD_DEFINED__ */


#ifndef __ISXMLAttributes_FWD_DEFINED__
#define __ISXMLAttributes_FWD_DEFINED__
typedef interface ISXMLAttributes ISXMLAttributes;
#endif 	/* __ISXMLAttributes_FWD_DEFINED__ */


#ifndef __IGeneralPlugin_FWD_DEFINED__
#define __IGeneralPlugin_FWD_DEFINED__
typedef interface IGeneralPlugin IGeneralPlugin;
#endif 	/* __IGeneralPlugin_FWD_DEFINED__ */


#ifndef __ICommandPlugin_FWD_DEFINED__
#define __ICommandPlugin_FWD_DEFINED__
typedef interface ICommandPlugin ICommandPlugin;
#endif 	/* __ICommandPlugin_FWD_DEFINED__ */


#ifndef __IExecutePlugin_FWD_DEFINED__
#define __IExecutePlugin_FWD_DEFINED__
typedef interface IExecutePlugin IExecutePlugin;
#endif 	/* __IExecutePlugin_FWD_DEFINED__ */


#ifndef __ISToolbar_FWD_DEFINED__
#define __ISToolbar_FWD_DEFINED__
typedef interface ISToolbar ISToolbar;
#endif 	/* __ISToolbar_FWD_DEFINED__ */


#ifndef __IPluginWindow_FWD_DEFINED__
#define __IPluginWindow_FWD_DEFINED__
typedef interface IPluginWindow IPluginWindow;
#endif 	/* __IPluginWindow_FWD_DEFINED__ */


#ifndef __IPluginWindowOwner_FWD_DEFINED__
#define __IPluginWindowOwner_FWD_DEFINED__
typedef interface IPluginWindowOwner IPluginWindowOwner;
#endif 	/* __IPluginWindowOwner_FWD_DEFINED__ */


#ifndef __ILibraryBuilderPlugin_FWD_DEFINED__
#define __ILibraryBuilderPlugin_FWD_DEFINED__
typedef interface ILibraryBuilderPlugin ILibraryBuilderPlugin;
#endif 	/* __ILibraryBuilderPlugin_FWD_DEFINED__ */


#ifndef __IImageServicePlugin_FWD_DEFINED__
#define __IImageServicePlugin_FWD_DEFINED__
typedef interface IImageServicePlugin IImageServicePlugin;
#endif 	/* __IImageServicePlugin_FWD_DEFINED__ */


#ifndef __IDownloadPreviewSite_FWD_DEFINED__
#define __IDownloadPreviewSite_FWD_DEFINED__
typedef interface IDownloadPreviewSite IDownloadPreviewSite;
#endif 	/* __IDownloadPreviewSite_FWD_DEFINED__ */


#ifndef __IDownloadPreviewPlugin_FWD_DEFINED__
#define __IDownloadPreviewPlugin_FWD_DEFINED__
typedef interface IDownloadPreviewPlugin IDownloadPreviewPlugin;
#endif 	/* __IDownloadPreviewPlugin_FWD_DEFINED__ */


#ifndef __IAudioVisPlugin_FWD_DEFINED__
#define __IAudioVisPlugin_FWD_DEFINED__
typedef interface IAudioVisPlugin IAudioVisPlugin;
#endif 	/* __IAudioVisPlugin_FWD_DEFINED__ */


#ifndef __IMediaPlayer_FWD_DEFINED__
#define __IMediaPlayer_FWD_DEFINED__
typedef interface IMediaPlayer IMediaPlayer;
#endif 	/* __IMediaPlayer_FWD_DEFINED__ */


#ifndef __IWrappedPluginControl_FWD_DEFINED__
#define __IWrappedPluginControl_FWD_DEFINED__
typedef interface IWrappedPluginControl IWrappedPluginControl;
#endif 	/* __IWrappedPluginControl_FWD_DEFINED__ */


#ifndef __IUserInterface_FWD_DEFINED__
#define __IUserInterface_FWD_DEFINED__
typedef interface IUserInterface IUserInterface;
#endif 	/* __IUserInterface_FWD_DEFINED__ */


#ifndef __ILibrary_FWD_DEFINED__
#define __ILibrary_FWD_DEFINED__
typedef interface ILibrary ILibrary;
#endif 	/* __ILibrary_FWD_DEFINED__ */


#ifndef __ISMenu_FWD_DEFINED__
#define __ISMenu_FWD_DEFINED__
typedef interface ISMenu ISMenu;
#endif 	/* __ISMenu_FWD_DEFINED__ */


#ifndef __ISToolbarItem_FWD_DEFINED__
#define __ISToolbarItem_FWD_DEFINED__
typedef interface ISToolbarItem ISToolbarItem;
#endif 	/* __ISToolbarItem_FWD_DEFINED__ */


#ifndef __IGenericView_FWD_DEFINED__
#define __IGenericView_FWD_DEFINED__
typedef interface IGenericView IGenericView;
#endif 	/* __IGenericView_FWD_DEFINED__ */


#ifndef __ILibraryFile_FWD_DEFINED__
#define __ILibraryFile_FWD_DEFINED__
typedef interface ILibraryFile ILibraryFile;
#endif 	/* __ILibraryFile_FWD_DEFINED__ */


#ifndef __ILibraryFiles_FWD_DEFINED__
#define __ILibraryFiles_FWD_DEFINED__
typedef interface ILibraryFiles ILibraryFiles;
#endif 	/* __ILibraryFiles_FWD_DEFINED__ */


#ifndef __ILibraryFolder_FWD_DEFINED__
#define __ILibraryFolder_FWD_DEFINED__
typedef interface ILibraryFolder ILibraryFolder;
#endif 	/* __ILibraryFolder_FWD_DEFINED__ */


#ifndef __ILibraryFolders_FWD_DEFINED__
#define __ILibraryFolders_FWD_DEFINED__
typedef interface ILibraryFolders ILibraryFolders;
#endif 	/* __ILibraryFolders_FWD_DEFINED__ */


#ifndef __ICollectionHtmlView_FWD_DEFINED__
#define __ICollectionHtmlView_FWD_DEFINED__
typedef interface ICollectionHtmlView ICollectionHtmlView;
#endif 	/* __ICollectionHtmlView_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __Shareaza_LIBRARY_DEFINED__
#define __Shareaza_LIBRARY_DEFINED__

/* library Shareaza */
/* [version][uuid] */ 


typedef 
enum __MIDL___MIDL_itf_Shareaza_0000_0001
    {	TSUNKNOWN	= 0,
	TSTRUE	= TSUNKNOWN + 1,
	TSFALSE	= TSTRUE + 1
    } 	STRISTATE;






typedef 
enum __MIDL___MIDL_itf_Shareaza_0000_0002
    {	xmlNode	= 0,
	xmlElement	= xmlNode + 1,
	xmlAttribute	= xmlElement + 1
    } 	SXMLNodeType;


typedef struct __MIDL___MIDL_itf_Shareaza_0125_0001
    {
    int cbSize;
    int nFlags;
    int nWidth;
    int nHeight;
    int nComponents;
    int nQuality;
    } 	IMAGESERVICEDATA;

typedef 
enum __MIDL___MIDL_itf_Shareaza_0125_0002
    {	IMAGESERVICE_SCANONLY	= 0x1,
	IMAGESERVICE_PARTIAL_IN	= 0x2,
	IMAGESERVICE_PARTIAL_OUT	= 0x4
    } 	IMAGESERVICEFLAGS;


typedef 
enum __MIDL___MIDL_itf_Shareaza_0128_0001
    {	smsNull	= 0,
	smsOpen	= smsNull + 1,
	smsPaused	= smsOpen + 1,
	smsPlaying	= smsPaused + 1
    } 	MediaState;

typedef 
enum __MIDL___MIDL_itf_Shareaza_0128_0002
    {	smaDefault	= 0,
	smzDistort	= -1,
	smzFill	= 0,
	smzOne	= 1,
	smzDouble	= 2
    } 	MediaZoom;

typedef struct __MIDL___MIDL_itf_Shareaza_0129_0001
    {
    DWORD nTime;
    BYTE nWaveform[ 2 ][ 512 ];
    BYTE nSpectrum[ 2 ][ 512 ];
    } 	SHAREAZAVISCHUNK;








typedef 
enum __MIDL___MIDL_itf_Shareaza_0133_0001
    {	mnuMenu	= 0,
	mnuSeparator	= mnuMenu + 1,
	mnuCommand	= mnuSeparator + 1
    } 	SMenuType;

typedef 
enum __MIDL___MIDL_itf_Shareaza_0134_0001
    {	tbSeparator	= 0,
	tbButton	= tbSeparator + 1,
	tbControl	= tbButton + 1
    } 	SToolbarType;






DEFINE_GUID(LIBID_Shareaza,0xE3481FE3,0xE062,0x4E1C,0xA2,0x3A,0x62,0xA6,0xD1,0x3C,0xBF,0xB8);

#ifndef __IApplication_INTERFACE_DEFINED__
#define __IApplication_INTERFACE_DEFINED__

/* interface IApplication */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_IApplication,0x8EBD0B6F,0x7BC4,0x44d1,0xBE,0xC1,0x03,0xE2,0x2D,0xC2,0xA5,0x87);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8EBD0B6F-7BC4-44d1-BEC1-03E22DC2A587")
    IApplication : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Version( 
            /* [retval][out] */ BSTR *psVersion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CheckVersion( 
            /* [in] */ BSTR sVersion) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CreateXML( 
            /* [retval][out] */ ISXMLElement **ppXML) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UserInterface( 
            /* [retval][out] */ IUserInterface **ppUserInterface) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Library( 
            /* [retval][out] */ ILibrary **ppLibrary) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IApplication * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IApplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IApplication * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IApplication * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IApplication * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IApplication * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            IApplication * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            IApplication * This,
            /* [retval][out] */ BSTR *psVersion);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CheckVersion )( 
            IApplication * This,
            /* [in] */ BSTR sVersion);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CreateXML )( 
            IApplication * This,
            /* [retval][out] */ ISXMLElement **ppXML);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UserInterface )( 
            IApplication * This,
            /* [retval][out] */ IUserInterface **ppUserInterface);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Library )( 
            IApplication * This,
            /* [retval][out] */ ILibrary **ppLibrary);
        
        END_INTERFACE
    } IApplicationVtbl;

    interface IApplication
    {
        CONST_VTBL struct IApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IApplication_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IApplication_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IApplication_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IApplication_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IApplication_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define IApplication_get_Version(This,psVersion)	\
    (This)->lpVtbl -> get_Version(This,psVersion)

#define IApplication_CheckVersion(This,sVersion)	\
    (This)->lpVtbl -> CheckVersion(This,sVersion)

#define IApplication_CreateXML(This,ppXML)	\
    (This)->lpVtbl -> CreateXML(This,ppXML)

#define IApplication_get_UserInterface(This,ppUserInterface)	\
    (This)->lpVtbl -> get_UserInterface(This,ppUserInterface)

#define IApplication_get_Library(This,ppLibrary)	\
    (This)->lpVtbl -> get_Library(This,ppLibrary)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IApplication_get_Application_Proxy( 
    IApplication * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB IApplication_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IApplication_get_Version_Proxy( 
    IApplication * This,
    /* [retval][out] */ BSTR *psVersion);


void __RPC_STUB IApplication_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IApplication_CheckVersion_Proxy( 
    IApplication * This,
    /* [in] */ BSTR sVersion);


void __RPC_STUB IApplication_CheckVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IApplication_CreateXML_Proxy( 
    IApplication * This,
    /* [retval][out] */ ISXMLElement **ppXML);


void __RPC_STUB IApplication_CreateXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IApplication_get_UserInterface_Proxy( 
    IApplication * This,
    /* [retval][out] */ IUserInterface **ppUserInterface);


void __RPC_STUB IApplication_get_UserInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IApplication_get_Library_Proxy( 
    IApplication * This,
    /* [retval][out] */ ILibrary **ppLibrary);


void __RPC_STUB IApplication_get_Library_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IApplication_INTERFACE_DEFINED__ */


#ifndef __ISXMLNode_INTERFACE_DEFINED__
#define __ISXMLNode_INTERFACE_DEFINED__

/* interface ISXMLNode */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISXMLNode,0xA0F89545,0xDAD8,0x4441,0x9D,0xF4,0xBC,0xB1,0x06,0xB1,0x22,0x34);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0F89545-DAD8-4441-9DF4-BCB106B12234")
    ISXMLNode : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ ISXMLElement **ppParent) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ SXMLNodeType *pnType) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_AsNode( 
            /* [retval][out] */ ISXMLNode **ppNode) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_AsElement( 
            /* [retval][out] */ ISXMLNode **ppElement) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_AsAttribute( 
            /* [retval][out] */ ISXMLNode **ppAttribute) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *psName) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR sName) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Value( 
            /* [retval][out] */ BSTR *psValue) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Value( 
            /* [in] */ BSTR sValue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE IsNamed( 
            /* [in] */ BSTR sName,
            /* [retval][out] */ VARIANT_BOOL *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISXMLNodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISXMLNode * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISXMLNode * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISXMLNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISXMLNode * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISXMLNode * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISXMLNode * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISXMLNode * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            ISXMLNode * This,
            /* [retval][out] */ ISXMLElement **ppParent);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISXMLNode * This,
            /* [retval][out] */ SXMLNodeType *pnType);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsNode )( 
            ISXMLNode * This,
            /* [retval][out] */ ISXMLNode **ppNode);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsElement )( 
            ISXMLNode * This,
            /* [retval][out] */ ISXMLNode **ppElement);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsAttribute )( 
            ISXMLNode * This,
            /* [retval][out] */ ISXMLNode **ppAttribute);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISXMLNode * This,
            /* [retval][out] */ BSTR *psName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISXMLNode * This,
            /* [in] */ BSTR sName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISXMLNode * This,
            /* [retval][out] */ BSTR *psValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISXMLNode * This,
            /* [in] */ BSTR sValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISXMLNode * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *IsNamed )( 
            ISXMLNode * This,
            /* [in] */ BSTR sName,
            /* [retval][out] */ VARIANT_BOOL *pbResult);
        
        END_INTERFACE
    } ISXMLNodeVtbl;

    interface ISXMLNode
    {
        CONST_VTBL struct ISXMLNodeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISXMLNode_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISXMLNode_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISXMLNode_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISXMLNode_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISXMLNode_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISXMLNode_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISXMLNode_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISXMLNode_get_Parent(This,ppParent)	\
    (This)->lpVtbl -> get_Parent(This,ppParent)

#define ISXMLNode_get_Type(This,pnType)	\
    (This)->lpVtbl -> get_Type(This,pnType)

#define ISXMLNode_get_AsNode(This,ppNode)	\
    (This)->lpVtbl -> get_AsNode(This,ppNode)

#define ISXMLNode_get_AsElement(This,ppElement)	\
    (This)->lpVtbl -> get_AsElement(This,ppElement)

#define ISXMLNode_get_AsAttribute(This,ppAttribute)	\
    (This)->lpVtbl -> get_AsAttribute(This,ppAttribute)

#define ISXMLNode_get_Name(This,psName)	\
    (This)->lpVtbl -> get_Name(This,psName)

#define ISXMLNode_put_Name(This,sName)	\
    (This)->lpVtbl -> put_Name(This,sName)

#define ISXMLNode_get_Value(This,psValue)	\
    (This)->lpVtbl -> get_Value(This,psValue)

#define ISXMLNode_put_Value(This,sValue)	\
    (This)->lpVtbl -> put_Value(This,sValue)

#define ISXMLNode_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ISXMLNode_IsNamed(This,sName,pbResult)	\
    (This)->lpVtbl -> IsNamed(This,sName,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_get_Parent_Proxy( 
    ISXMLNode * This,
    /* [retval][out] */ ISXMLElement **ppParent);


void __RPC_STUB ISXMLNode_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_get_Type_Proxy( 
    ISXMLNode * This,
    /* [retval][out] */ SXMLNodeType *pnType);


void __RPC_STUB ISXMLNode_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_get_AsNode_Proxy( 
    ISXMLNode * This,
    /* [retval][out] */ ISXMLNode **ppNode);


void __RPC_STUB ISXMLNode_get_AsNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_get_AsElement_Proxy( 
    ISXMLNode * This,
    /* [retval][out] */ ISXMLNode **ppElement);


void __RPC_STUB ISXMLNode_get_AsElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_get_AsAttribute_Proxy( 
    ISXMLNode * This,
    /* [retval][out] */ ISXMLNode **ppAttribute);


void __RPC_STUB ISXMLNode_get_AsAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_get_Name_Proxy( 
    ISXMLNode * This,
    /* [retval][out] */ BSTR *psName);


void __RPC_STUB ISXMLNode_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_put_Name_Proxy( 
    ISXMLNode * This,
    /* [in] */ BSTR sName);


void __RPC_STUB ISXMLNode_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_get_Value_Proxy( 
    ISXMLNode * This,
    /* [retval][out] */ BSTR *psValue);


void __RPC_STUB ISXMLNode_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_put_Value_Proxy( 
    ISXMLNode * This,
    /* [in] */ BSTR sValue);


void __RPC_STUB ISXMLNode_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_Delete_Proxy( 
    ISXMLNode * This);


void __RPC_STUB ISXMLNode_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLNode_IsNamed_Proxy( 
    ISXMLNode * This,
    /* [in] */ BSTR sName,
    /* [retval][out] */ VARIANT_BOOL *pbResult);


void __RPC_STUB ISXMLNode_IsNamed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISXMLNode_INTERFACE_DEFINED__ */


#ifndef __ISXMLElement_INTERFACE_DEFINED__
#define __ISXMLElement_INTERFACE_DEFINED__

/* interface ISXMLElement */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISXMLElement,0x5198A470,0xF9EE,0x49eb,0x94,0x8C,0xF8,0x17,0x66,0x10,0xA8,0xB2);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5198A470-F9EE-49eb-948C-F8176610A8B2")
    ISXMLElement : public ISXMLNode
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Elements( 
            /* [retval][out] */ ISXMLElements **ppElements) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Attributes( 
            /* [retval][out] */ ISXMLAttributes **ppAttributes) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ ISXMLElement **ppClone) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ToString( 
            /* [retval][out] */ BSTR *psValue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE ToStringEx( 
            /* [in] */ VARIANT_BOOL bHeader,
            /* [in] */ VARIANT_BOOL bNewlines,
            /* [retval][out] */ BSTR *psValue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FromString( 
            /* [in] */ BSTR sXML,
            /* [retval][out] */ ISXMLElement **ppElement) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetWords( 
            /* [retval][out] */ BSTR *psWords) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISXMLElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISXMLElement * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISXMLElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISXMLElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISXMLElement * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISXMLElement * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISXMLElement * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISXMLElement * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            ISXMLElement * This,
            /* [retval][out] */ ISXMLElement **ppParent);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISXMLElement * This,
            /* [retval][out] */ SXMLNodeType *pnType);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsNode )( 
            ISXMLElement * This,
            /* [retval][out] */ ISXMLNode **ppNode);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsElement )( 
            ISXMLElement * This,
            /* [retval][out] */ ISXMLNode **ppElement);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsAttribute )( 
            ISXMLElement * This,
            /* [retval][out] */ ISXMLNode **ppAttribute);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISXMLElement * This,
            /* [retval][out] */ BSTR *psName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISXMLElement * This,
            /* [in] */ BSTR sName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISXMLElement * This,
            /* [retval][out] */ BSTR *psValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISXMLElement * This,
            /* [in] */ BSTR sValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISXMLElement * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *IsNamed )( 
            ISXMLElement * This,
            /* [in] */ BSTR sName,
            /* [retval][out] */ VARIANT_BOOL *pbResult);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Elements )( 
            ISXMLElement * This,
            /* [retval][out] */ ISXMLElements **ppElements);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            ISXMLElement * This,
            /* [retval][out] */ ISXMLAttributes **ppAttributes);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Detach )( 
            ISXMLElement * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISXMLElement * This,
            /* [retval][out] */ ISXMLElement **ppClone);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ToString )( 
            ISXMLElement * This,
            /* [retval][out] */ BSTR *psValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *ToStringEx )( 
            ISXMLElement * This,
            /* [in] */ VARIANT_BOOL bHeader,
            /* [in] */ VARIANT_BOOL bNewlines,
            /* [retval][out] */ BSTR *psValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FromString )( 
            ISXMLElement * This,
            /* [in] */ BSTR sXML,
            /* [retval][out] */ ISXMLElement **ppElement);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetWords )( 
            ISXMLElement * This,
            /* [retval][out] */ BSTR *psWords);
        
        END_INTERFACE
    } ISXMLElementVtbl;

    interface ISXMLElement
    {
        CONST_VTBL struct ISXMLElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISXMLElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISXMLElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISXMLElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISXMLElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISXMLElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISXMLElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISXMLElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISXMLElement_get_Parent(This,ppParent)	\
    (This)->lpVtbl -> get_Parent(This,ppParent)

#define ISXMLElement_get_Type(This,pnType)	\
    (This)->lpVtbl -> get_Type(This,pnType)

#define ISXMLElement_get_AsNode(This,ppNode)	\
    (This)->lpVtbl -> get_AsNode(This,ppNode)

#define ISXMLElement_get_AsElement(This,ppElement)	\
    (This)->lpVtbl -> get_AsElement(This,ppElement)

#define ISXMLElement_get_AsAttribute(This,ppAttribute)	\
    (This)->lpVtbl -> get_AsAttribute(This,ppAttribute)

#define ISXMLElement_get_Name(This,psName)	\
    (This)->lpVtbl -> get_Name(This,psName)

#define ISXMLElement_put_Name(This,sName)	\
    (This)->lpVtbl -> put_Name(This,sName)

#define ISXMLElement_get_Value(This,psValue)	\
    (This)->lpVtbl -> get_Value(This,psValue)

#define ISXMLElement_put_Value(This,sValue)	\
    (This)->lpVtbl -> put_Value(This,sValue)

#define ISXMLElement_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ISXMLElement_IsNamed(This,sName,pbResult)	\
    (This)->lpVtbl -> IsNamed(This,sName,pbResult)


#define ISXMLElement_get_Elements(This,ppElements)	\
    (This)->lpVtbl -> get_Elements(This,ppElements)

#define ISXMLElement_get_Attributes(This,ppAttributes)	\
    (This)->lpVtbl -> get_Attributes(This,ppAttributes)

#define ISXMLElement_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#define ISXMLElement_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#define ISXMLElement_ToString(This,psValue)	\
    (This)->lpVtbl -> ToString(This,psValue)

#define ISXMLElement_ToStringEx(This,bHeader,bNewlines,psValue)	\
    (This)->lpVtbl -> ToStringEx(This,bHeader,bNewlines,psValue)

#define ISXMLElement_FromString(This,sXML,ppElement)	\
    (This)->lpVtbl -> FromString(This,sXML,ppElement)

#define ISXMLElement_GetWords(This,psWords)	\
    (This)->lpVtbl -> GetWords(This,psWords)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_get_Elements_Proxy( 
    ISXMLElement * This,
    /* [retval][out] */ ISXMLElements **ppElements);


void __RPC_STUB ISXMLElement_get_Elements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_get_Attributes_Proxy( 
    ISXMLElement * This,
    /* [retval][out] */ ISXMLAttributes **ppAttributes);


void __RPC_STUB ISXMLElement_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_Detach_Proxy( 
    ISXMLElement * This);


void __RPC_STUB ISXMLElement_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_Clone_Proxy( 
    ISXMLElement * This,
    /* [retval][out] */ ISXMLElement **ppClone);


void __RPC_STUB ISXMLElement_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_ToString_Proxy( 
    ISXMLElement * This,
    /* [retval][out] */ BSTR *psValue);


void __RPC_STUB ISXMLElement_ToString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_ToStringEx_Proxy( 
    ISXMLElement * This,
    /* [in] */ VARIANT_BOOL bHeader,
    /* [in] */ VARIANT_BOOL bNewlines,
    /* [retval][out] */ BSTR *psValue);


void __RPC_STUB ISXMLElement_ToStringEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_FromString_Proxy( 
    ISXMLElement * This,
    /* [in] */ BSTR sXML,
    /* [retval][out] */ ISXMLElement **ppElement);


void __RPC_STUB ISXMLElement_FromString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElement_GetWords_Proxy( 
    ISXMLElement * This,
    /* [retval][out] */ BSTR *psWords);


void __RPC_STUB ISXMLElement_GetWords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISXMLElement_INTERFACE_DEFINED__ */


#ifndef __ISXMLElements_INTERFACE_DEFINED__
#define __ISXMLElements_INTERFACE_DEFINED__

/* interface ISXMLElements */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISXMLElements,0x10BF271C,0x85A3,0x4ad4,0x89,0x30,0xCC,0x0E,0x3C,0xEE,0xAD,0xA4);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10BF271C-85A3-4ad4-8930-CC0E3CEEADA4")
    ISXMLElements : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISXMLElement **ppElement) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ BSTR strName,
            /* [retval][out] */ ISXMLElement **ppElement) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Attach( 
            /* [in] */ ISXMLElement *pElement) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ ISXMLElement **ppElement) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ByName( 
            /* [in] */ BSTR sName,
            /* [retval][out] */ ISXMLElement **ppElement) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISXMLElementsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISXMLElements * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISXMLElements * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISXMLElements * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISXMLElements * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISXMLElements * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISXMLElements * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISXMLElements * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISXMLElements * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISXMLElements * This,
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISXMLElement **ppElement);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISXMLElements * This,
            /* [retval][out] */ LONG *pnCount);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Create )( 
            ISXMLElements * This,
            /* [in] */ BSTR strName,
            /* [retval][out] */ ISXMLElement **ppElement);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Attach )( 
            ISXMLElements * This,
            /* [in] */ ISXMLElement *pElement);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *RemoveAll )( 
            ISXMLElements * This);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            ISXMLElements * This,
            /* [retval][out] */ ISXMLElement **ppElement);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ByName )( 
            ISXMLElements * This,
            /* [in] */ BSTR sName,
            /* [retval][out] */ ISXMLElement **ppElement);
        
        END_INTERFACE
    } ISXMLElementsVtbl;

    interface ISXMLElements
    {
        CONST_VTBL struct ISXMLElementsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISXMLElements_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISXMLElements_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISXMLElements_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISXMLElements_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISXMLElements_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISXMLElements_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISXMLElements_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISXMLElements_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ISXMLElements_get_Item(This,vIndex,ppElement)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppElement)

#define ISXMLElements_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#define ISXMLElements_Create(This,strName,ppElement)	\
    (This)->lpVtbl -> Create(This,strName,ppElement)

#define ISXMLElements_Attach(This,pElement)	\
    (This)->lpVtbl -> Attach(This,pElement)

#define ISXMLElements_RemoveAll(This)	\
    (This)->lpVtbl -> RemoveAll(This)

#define ISXMLElements_get_First(This,ppElement)	\
    (This)->lpVtbl -> get_First(This,ppElement)

#define ISXMLElements_get_ByName(This,sName,ppElement)	\
    (This)->lpVtbl -> get_ByName(This,sName,ppElement)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_get__NewEnum_Proxy( 
    ISXMLElements * This,
    /* [retval][out] */ IUnknown **ppEnum);


void __RPC_STUB ISXMLElements_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_get_Item_Proxy( 
    ISXMLElements * This,
    /* [in] */ VARIANT vIndex,
    /* [retval][out] */ ISXMLElement **ppElement);


void __RPC_STUB ISXMLElements_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_get_Count_Proxy( 
    ISXMLElements * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB ISXMLElements_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_Create_Proxy( 
    ISXMLElements * This,
    /* [in] */ BSTR strName,
    /* [retval][out] */ ISXMLElement **ppElement);


void __RPC_STUB ISXMLElements_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_Attach_Proxy( 
    ISXMLElements * This,
    /* [in] */ ISXMLElement *pElement);


void __RPC_STUB ISXMLElements_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_RemoveAll_Proxy( 
    ISXMLElements * This);


void __RPC_STUB ISXMLElements_RemoveAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_get_First_Proxy( 
    ISXMLElements * This,
    /* [retval][out] */ ISXMLElement **ppElement);


void __RPC_STUB ISXMLElements_get_First_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLElements_get_ByName_Proxy( 
    ISXMLElements * This,
    /* [in] */ BSTR sName,
    /* [retval][out] */ ISXMLElement **ppElement);


void __RPC_STUB ISXMLElements_get_ByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISXMLElements_INTERFACE_DEFINED__ */


#ifndef __ISXMLAttribute_INTERFACE_DEFINED__
#define __ISXMLAttribute_INTERFACE_DEFINED__

/* interface ISXMLAttribute */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISXMLAttribute,0x6D4598A7,0x26A1,0x4990,0xBA,0x60,0xDE,0x0E,0x21,0x2A,0xF9,0x3C);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6D4598A7-26A1-4990-BA60-DE0E212AF93C")
    ISXMLAttribute : public ISXMLNode
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Detach( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ ISXMLAttribute **ppClone) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISXMLAttributeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISXMLAttribute * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISXMLAttribute * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISXMLAttribute * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISXMLAttribute * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISXMLAttribute * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISXMLAttribute * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISXMLAttribute * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            ISXMLAttribute * This,
            /* [retval][out] */ ISXMLElement **ppParent);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISXMLAttribute * This,
            /* [retval][out] */ SXMLNodeType *pnType);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsNode )( 
            ISXMLAttribute * This,
            /* [retval][out] */ ISXMLNode **ppNode);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsElement )( 
            ISXMLAttribute * This,
            /* [retval][out] */ ISXMLNode **ppElement);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AsAttribute )( 
            ISXMLAttribute * This,
            /* [retval][out] */ ISXMLNode **ppAttribute);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISXMLAttribute * This,
            /* [retval][out] */ BSTR *psName);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            ISXMLAttribute * This,
            /* [in] */ BSTR sName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISXMLAttribute * This,
            /* [retval][out] */ BSTR *psValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISXMLAttribute * This,
            /* [in] */ BSTR sValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ISXMLAttribute * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *IsNamed )( 
            ISXMLAttribute * This,
            /* [in] */ BSTR sName,
            /* [retval][out] */ VARIANT_BOOL *pbResult);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Detach )( 
            ISXMLAttribute * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISXMLAttribute * This,
            /* [retval][out] */ ISXMLAttribute **ppClone);
        
        END_INTERFACE
    } ISXMLAttributeVtbl;

    interface ISXMLAttribute
    {
        CONST_VTBL struct ISXMLAttributeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISXMLAttribute_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISXMLAttribute_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISXMLAttribute_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISXMLAttribute_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISXMLAttribute_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISXMLAttribute_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISXMLAttribute_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISXMLAttribute_get_Parent(This,ppParent)	\
    (This)->lpVtbl -> get_Parent(This,ppParent)

#define ISXMLAttribute_get_Type(This,pnType)	\
    (This)->lpVtbl -> get_Type(This,pnType)

#define ISXMLAttribute_get_AsNode(This,ppNode)	\
    (This)->lpVtbl -> get_AsNode(This,ppNode)

#define ISXMLAttribute_get_AsElement(This,ppElement)	\
    (This)->lpVtbl -> get_AsElement(This,ppElement)

#define ISXMLAttribute_get_AsAttribute(This,ppAttribute)	\
    (This)->lpVtbl -> get_AsAttribute(This,ppAttribute)

#define ISXMLAttribute_get_Name(This,psName)	\
    (This)->lpVtbl -> get_Name(This,psName)

#define ISXMLAttribute_put_Name(This,sName)	\
    (This)->lpVtbl -> put_Name(This,sName)

#define ISXMLAttribute_get_Value(This,psValue)	\
    (This)->lpVtbl -> get_Value(This,psValue)

#define ISXMLAttribute_put_Value(This,sValue)	\
    (This)->lpVtbl -> put_Value(This,sValue)

#define ISXMLAttribute_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ISXMLAttribute_IsNamed(This,sName,pbResult)	\
    (This)->lpVtbl -> IsNamed(This,sName,pbResult)


#define ISXMLAttribute_Detach(This)	\
    (This)->lpVtbl -> Detach(This)

#define ISXMLAttribute_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLAttribute_Detach_Proxy( 
    ISXMLAttribute * This);


void __RPC_STUB ISXMLAttribute_Detach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLAttribute_Clone_Proxy( 
    ISXMLAttribute * This,
    /* [retval][out] */ ISXMLAttribute **ppClone);


void __RPC_STUB ISXMLAttribute_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISXMLAttribute_INTERFACE_DEFINED__ */


#ifndef __ISXMLAttributes_INTERFACE_DEFINED__
#define __ISXMLAttributes_INTERFACE_DEFINED__

/* interface ISXMLAttributes */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISXMLAttributes,0x8E03E1BF,0xCCC0,0x4616,0x9C,0x0D,0x82,0x04,0xA8,0x3B,0xAE,0xB4);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8E03E1BF-CCC0-4616-9C0D-8204A83BAEB4")
    ISXMLAttributes : public IDispatch
    {
    public:
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISXMLAttribute **ppAttribute) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ BSTR strName,
            /* [in] */ BSTR strValue) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ BSTR strName,
            /* [retval][out] */ ISXMLAttribute **ppAttribute) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Attach( 
            /* [in] */ ISXMLAttribute *pAttribute) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RemoveAll( void) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ByName( 
            /* [in] */ BSTR sName,
            /* [retval][out] */ ISXMLAttribute **ppAttribute) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Get( 
            /* [in] */ BSTR sName,
            /* [retval][out] */ BSTR *psValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISXMLAttributesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISXMLAttributes * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISXMLAttributes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISXMLAttributes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISXMLAttributes * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISXMLAttributes * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISXMLAttributes * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISXMLAttributes * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISXMLAttributes * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISXMLAttributes * This,
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISXMLAttribute **ppAttribute);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISXMLAttributes * This,
            /* [retval][out] */ LONG *pnCount);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Add )( 
            ISXMLAttributes * This,
            /* [in] */ BSTR strName,
            /* [in] */ BSTR strValue);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Create )( 
            ISXMLAttributes * This,
            /* [in] */ BSTR strName,
            /* [retval][out] */ ISXMLAttribute **ppAttribute);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Attach )( 
            ISXMLAttributes * This,
            /* [in] */ ISXMLAttribute *pAttribute);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *RemoveAll )( 
            ISXMLAttributes * This);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ByName )( 
            ISXMLAttributes * This,
            /* [in] */ BSTR sName,
            /* [retval][out] */ ISXMLAttribute **ppAttribute);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Get )( 
            ISXMLAttributes * This,
            /* [in] */ BSTR sName,
            /* [retval][out] */ BSTR *psValue);
        
        END_INTERFACE
    } ISXMLAttributesVtbl;

    interface ISXMLAttributes
    {
        CONST_VTBL struct ISXMLAttributesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISXMLAttributes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISXMLAttributes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISXMLAttributes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISXMLAttributes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISXMLAttributes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISXMLAttributes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISXMLAttributes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISXMLAttributes_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ISXMLAttributes_get_Item(This,vIndex,ppAttribute)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppAttribute)

#define ISXMLAttributes_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#define ISXMLAttributes_Add(This,strName,strValue)	\
    (This)->lpVtbl -> Add(This,strName,strValue)

#define ISXMLAttributes_Create(This,strName,ppAttribute)	\
    (This)->lpVtbl -> Create(This,strName,ppAttribute)

#define ISXMLAttributes_Attach(This,pAttribute)	\
    (This)->lpVtbl -> Attach(This,pAttribute)

#define ISXMLAttributes_RemoveAll(This)	\
    (This)->lpVtbl -> RemoveAll(This)

#define ISXMLAttributes_get_ByName(This,sName,ppAttribute)	\
    (This)->lpVtbl -> get_ByName(This,sName,ppAttribute)

#define ISXMLAttributes_get_Get(This,sName,psValue)	\
    (This)->lpVtbl -> get_Get(This,sName,psValue)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_get__NewEnum_Proxy( 
    ISXMLAttributes * This,
    /* [retval][out] */ IUnknown **ppEnum);


void __RPC_STUB ISXMLAttributes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_get_Item_Proxy( 
    ISXMLAttributes * This,
    /* [in] */ VARIANT vIndex,
    /* [retval][out] */ ISXMLAttribute **ppAttribute);


void __RPC_STUB ISXMLAttributes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_get_Count_Proxy( 
    ISXMLAttributes * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB ISXMLAttributes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_Add_Proxy( 
    ISXMLAttributes * This,
    /* [in] */ BSTR strName,
    /* [in] */ BSTR strValue);


void __RPC_STUB ISXMLAttributes_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_Create_Proxy( 
    ISXMLAttributes * This,
    /* [in] */ BSTR strName,
    /* [retval][out] */ ISXMLAttribute **ppAttribute);


void __RPC_STUB ISXMLAttributes_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_Attach_Proxy( 
    ISXMLAttributes * This,
    /* [in] */ ISXMLAttribute *pAttribute);


void __RPC_STUB ISXMLAttributes_Attach_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_RemoveAll_Proxy( 
    ISXMLAttributes * This);


void __RPC_STUB ISXMLAttributes_RemoveAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_get_ByName_Proxy( 
    ISXMLAttributes * This,
    /* [in] */ BSTR sName,
    /* [retval][out] */ ISXMLAttribute **ppAttribute);


void __RPC_STUB ISXMLAttributes_get_ByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISXMLAttributes_get_Get_Proxy( 
    ISXMLAttributes * This,
    /* [in] */ BSTR sName,
    /* [retval][out] */ BSTR *psValue);


void __RPC_STUB ISXMLAttributes_get_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISXMLAttributes_INTERFACE_DEFINED__ */


#ifndef __IGeneralPlugin_INTERFACE_DEFINED__
#define __IGeneralPlugin_INTERFACE_DEFINED__

/* interface IGeneralPlugin */
/* [object][uuid] */ 


DEFINE_GUID(IID_IGeneralPlugin,0xD1B5D3A4,0xB890,0x470a,0xA3,0xFF,0x97,0x00,0xF3,0xC2,0xA0,0x63);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D1B5D3A4-B890-470a-A3FF-9700F3C2A063")
    IGeneralPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetApplication( 
            /* [in] */ IApplication *pApplication) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryCapabilities( 
            /* [out] */ DWORD *pnCaps) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSkinChanged( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGeneralPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGeneralPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGeneralPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGeneralPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetApplication )( 
            IGeneralPlugin * This,
            /* [in] */ IApplication *pApplication);
        
        HRESULT ( STDMETHODCALLTYPE *QueryCapabilities )( 
            IGeneralPlugin * This,
            /* [out] */ DWORD *pnCaps);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            IGeneralPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnSkinChanged )( 
            IGeneralPlugin * This);
        
        END_INTERFACE
    } IGeneralPluginVtbl;

    interface IGeneralPlugin
    {
        CONST_VTBL struct IGeneralPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGeneralPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGeneralPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGeneralPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGeneralPlugin_SetApplication(This,pApplication)	\
    (This)->lpVtbl -> SetApplication(This,pApplication)

#define IGeneralPlugin_QueryCapabilities(This,pnCaps)	\
    (This)->lpVtbl -> QueryCapabilities(This,pnCaps)

#define IGeneralPlugin_Configure(This)	\
    (This)->lpVtbl -> Configure(This)

#define IGeneralPlugin_OnSkinChanged(This)	\
    (This)->lpVtbl -> OnSkinChanged(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IGeneralPlugin_SetApplication_Proxy( 
    IGeneralPlugin * This,
    /* [in] */ IApplication *pApplication);


void __RPC_STUB IGeneralPlugin_SetApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGeneralPlugin_QueryCapabilities_Proxy( 
    IGeneralPlugin * This,
    /* [out] */ DWORD *pnCaps);


void __RPC_STUB IGeneralPlugin_QueryCapabilities_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGeneralPlugin_Configure_Proxy( 
    IGeneralPlugin * This);


void __RPC_STUB IGeneralPlugin_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGeneralPlugin_OnSkinChanged_Proxy( 
    IGeneralPlugin * This);


void __RPC_STUB IGeneralPlugin_OnSkinChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGeneralPlugin_INTERFACE_DEFINED__ */


#ifndef __ICommandPlugin_INTERFACE_DEFINED__
#define __ICommandPlugin_INTERFACE_DEFINED__

/* interface ICommandPlugin */
/* [object][uuid] */ 


DEFINE_GUID(IID_ICommandPlugin,0xCB25DAED,0xD745,0x45db,0x99,0x4E,0x32,0x63,0x9D,0x28,0x88,0xA9);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CB25DAED-D745-45db-994E-32639D2888A9")
    ICommandPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterCommands( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InsertCommands( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUpdate( 
            /* [in] */ UINT nCommandID,
            /* [out][in] */ STRISTATE *pbVisible,
            /* [out][in] */ STRISTATE *pbEnabled,
            /* [out][in] */ STRISTATE *pbChecked) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCommand( 
            /* [in] */ UINT nCommandID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICommandPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICommandPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICommandPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterCommands )( 
            ICommandPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *InsertCommands )( 
            ICommandPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnUpdate )( 
            ICommandPlugin * This,
            /* [in] */ UINT nCommandID,
            /* [out][in] */ STRISTATE *pbVisible,
            /* [out][in] */ STRISTATE *pbEnabled,
            /* [out][in] */ STRISTATE *pbChecked);
        
        HRESULT ( STDMETHODCALLTYPE *OnCommand )( 
            ICommandPlugin * This,
            /* [in] */ UINT nCommandID);
        
        END_INTERFACE
    } ICommandPluginVtbl;

    interface ICommandPlugin
    {
        CONST_VTBL struct ICommandPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandPlugin_RegisterCommands(This)	\
    (This)->lpVtbl -> RegisterCommands(This)

#define ICommandPlugin_InsertCommands(This)	\
    (This)->lpVtbl -> InsertCommands(This)

#define ICommandPlugin_OnUpdate(This,nCommandID,pbVisible,pbEnabled,pbChecked)	\
    (This)->lpVtbl -> OnUpdate(This,nCommandID,pbVisible,pbEnabled,pbChecked)

#define ICommandPlugin_OnCommand(This,nCommandID)	\
    (This)->lpVtbl -> OnCommand(This,nCommandID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandPlugin_RegisterCommands_Proxy( 
    ICommandPlugin * This);


void __RPC_STUB ICommandPlugin_RegisterCommands_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPlugin_InsertCommands_Proxy( 
    ICommandPlugin * This);


void __RPC_STUB ICommandPlugin_InsertCommands_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPlugin_OnUpdate_Proxy( 
    ICommandPlugin * This,
    /* [in] */ UINT nCommandID,
    /* [out][in] */ STRISTATE *pbVisible,
    /* [out][in] */ STRISTATE *pbEnabled,
    /* [out][in] */ STRISTATE *pbChecked);


void __RPC_STUB ICommandPlugin_OnUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPlugin_OnCommand_Proxy( 
    ICommandPlugin * This,
    /* [in] */ UINT nCommandID);


void __RPC_STUB ICommandPlugin_OnCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandPlugin_INTERFACE_DEFINED__ */


#ifndef __IExecutePlugin_INTERFACE_DEFINED__
#define __IExecutePlugin_INTERFACE_DEFINED__

/* interface IExecutePlugin */
/* [object][uuid] */ 


DEFINE_GUID(IID_IExecutePlugin,0x8E878640,0x37B5,0x44a3,0xA3,0x7C,0xFC,0x3B,0xF1,0xCC,0xF6,0xB6);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8E878640-37B5-44a3-A37C-FC3BF1CCF6B6")
    IExecutePlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnExecute( 
            /* [in] */ BSTR sFilePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnEnqueue( 
            /* [in] */ BSTR sFilePath) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IExecutePluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExecutePlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExecutePlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExecutePlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnExecute )( 
            IExecutePlugin * This,
            /* [in] */ BSTR sFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *OnEnqueue )( 
            IExecutePlugin * This,
            /* [in] */ BSTR sFilePath);
        
        END_INTERFACE
    } IExecutePluginVtbl;

    interface IExecutePlugin
    {
        CONST_VTBL struct IExecutePluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExecutePlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IExecutePlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IExecutePlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IExecutePlugin_OnExecute(This,sFilePath)	\
    (This)->lpVtbl -> OnExecute(This,sFilePath)

#define IExecutePlugin_OnEnqueue(This,sFilePath)	\
    (This)->lpVtbl -> OnEnqueue(This,sFilePath)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IExecutePlugin_OnExecute_Proxy( 
    IExecutePlugin * This,
    /* [in] */ BSTR sFilePath);


void __RPC_STUB IExecutePlugin_OnExecute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IExecutePlugin_OnEnqueue_Proxy( 
    IExecutePlugin * This,
    /* [in] */ BSTR sFilePath);


void __RPC_STUB IExecutePlugin_OnEnqueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IExecutePlugin_INTERFACE_DEFINED__ */


#ifndef __ISToolbar_INTERFACE_DEFINED__
#define __ISToolbar_INTERFACE_DEFINED__

/* interface ISToolbar */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISToolbar,0xE02F505E,0x9649,0x4eb1,0xAB,0x3F,0x56,0xFF,0xDF,0xF5,0xB9,0x2C);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E02F505E-9649-4eb1-AB3F-56FFDFF5B92C")
    ISToolbar : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UserInterface( 
            /* [retval][out] */ IUserInterface **ppUserInterface) = 0;
        
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISToolbarItem **ppItem) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InsertSeparator( 
            /* [in] */ LONG nPosition) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InsertButton( 
            /* [in] */ LONG nPosition,
            /* [in] */ LONG nCommandID,
            /* [in] */ BSTR sText,
            /* [retval][out] */ ISToolbarItem **ppItem) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISToolbarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISToolbar * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISToolbar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISToolbar * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISToolbar * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISToolbar * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISToolbar * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISToolbar * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ISToolbar * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UserInterface )( 
            ISToolbar * This,
            /* [retval][out] */ IUserInterface **ppUserInterface);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISToolbar * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISToolbar * This,
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISToolbarItem **ppItem);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISToolbar * This,
            /* [retval][out] */ LONG *pnCount);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InsertSeparator )( 
            ISToolbar * This,
            /* [in] */ LONG nPosition);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InsertButton )( 
            ISToolbar * This,
            /* [in] */ LONG nPosition,
            /* [in] */ LONG nCommandID,
            /* [in] */ BSTR sText,
            /* [retval][out] */ ISToolbarItem **ppItem);
        
        END_INTERFACE
    } ISToolbarVtbl;

    interface ISToolbar
    {
        CONST_VTBL struct ISToolbarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISToolbar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISToolbar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISToolbar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISToolbar_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISToolbar_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISToolbar_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISToolbar_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISToolbar_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ISToolbar_get_UserInterface(This,ppUserInterface)	\
    (This)->lpVtbl -> get_UserInterface(This,ppUserInterface)

#define ISToolbar_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ISToolbar_get_Item(This,vIndex,ppItem)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppItem)

#define ISToolbar_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#define ISToolbar_InsertSeparator(This,nPosition)	\
    (This)->lpVtbl -> InsertSeparator(This,nPosition)

#define ISToolbar_InsertButton(This,nPosition,nCommandID,sText,ppItem)	\
    (This)->lpVtbl -> InsertButton(This,nPosition,nCommandID,sText,ppItem)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbar_get_Application_Proxy( 
    ISToolbar * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ISToolbar_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbar_get_UserInterface_Proxy( 
    ISToolbar * This,
    /* [retval][out] */ IUserInterface **ppUserInterface);


void __RPC_STUB ISToolbar_get_UserInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbar_get__NewEnum_Proxy( 
    ISToolbar * This,
    /* [retval][out] */ IUnknown **ppEnum);


void __RPC_STUB ISToolbar_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbar_get_Item_Proxy( 
    ISToolbar * This,
    /* [in] */ VARIANT vIndex,
    /* [retval][out] */ ISToolbarItem **ppItem);


void __RPC_STUB ISToolbar_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbar_get_Count_Proxy( 
    ISToolbar * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB ISToolbar_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISToolbar_InsertSeparator_Proxy( 
    ISToolbar * This,
    /* [in] */ LONG nPosition);


void __RPC_STUB ISToolbar_InsertSeparator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISToolbar_InsertButton_Proxy( 
    ISToolbar * This,
    /* [in] */ LONG nPosition,
    /* [in] */ LONG nCommandID,
    /* [in] */ BSTR sText,
    /* [retval][out] */ ISToolbarItem **ppItem);


void __RPC_STUB ISToolbar_InsertButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISToolbar_INTERFACE_DEFINED__ */


#ifndef __IPluginWindow_INTERFACE_DEFINED__
#define __IPluginWindow_INTERFACE_DEFINED__

/* interface IPluginWindow */
/* [object][uuid] */ 


DEFINE_GUID(IID_IPluginWindow,0xADDB77AE,0x3483,0x4a15,0xB3,0x91,0xAE,0x31,0x56,0x3B,0xD7,0xE3);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ADDB77AE-3483-4a15-B391-AE31563BD7E3")
    IPluginWindow : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ListenForSingleMessage( 
            /* [in] */ UINT nMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ListenForMultipleMessages( 
            /* [in] */ SAFEARRAY *pMessages) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Create1( 
            /* [in] */ BSTR bsCaption,
            /* [in] */ HICON hIcon,
            /* [in] */ VARIANT_BOOL bPanel,
            /* [in] */ VARIANT_BOOL bTabbed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Create2( 
            /* [in] */ UINT nCommandID,
            /* [in] */ VARIANT_BOOL bPanel,
            /* [in] */ VARIANT_BOOL bTabbed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHwnd( 
            /* [out] */ HWND *phWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HandleMessage( 
            /* [out] */ LRESULT *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadState( 
            /* [in] */ VARIANT_BOOL bMaximise) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveState( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ThrowMenu( 
            /* [in] */ BSTR sName,
            /* [in] */ LONG nDefaultID,
            /* [in] */ POINT *pPoint) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddToolbar( 
            /* [in] */ BSTR sName,
            /* [in] */ LONG nPosition,
            /* [out] */ HWND *phWnd,
            /* [retval][out] */ ISToolbar **ppToolbar) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AdjustWindowRect( 
            /* [out][in] */ RECT *pRect,
            /* [in] */ VARIANT_BOOL bClientToWindow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPluginWindowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPluginWindow * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPluginWindow * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPluginWindow * This);
        
        HRESULT ( STDMETHODCALLTYPE *ListenForSingleMessage )( 
            IPluginWindow * This,
            /* [in] */ UINT nMessage);
        
        HRESULT ( STDMETHODCALLTYPE *ListenForMultipleMessages )( 
            IPluginWindow * This,
            /* [in] */ SAFEARRAY *pMessages);
        
        HRESULT ( STDMETHODCALLTYPE *Create1 )( 
            IPluginWindow * This,
            /* [in] */ BSTR bsCaption,
            /* [in] */ HICON hIcon,
            /* [in] */ VARIANT_BOOL bPanel,
            /* [in] */ VARIANT_BOOL bTabbed);
        
        HRESULT ( STDMETHODCALLTYPE *Create2 )( 
            IPluginWindow * This,
            /* [in] */ UINT nCommandID,
            /* [in] */ VARIANT_BOOL bPanel,
            /* [in] */ VARIANT_BOOL bTabbed);
        
        HRESULT ( STDMETHODCALLTYPE *GetHwnd )( 
            IPluginWindow * This,
            /* [out] */ HWND *phWnd);
        
        HRESULT ( STDMETHODCALLTYPE *HandleMessage )( 
            IPluginWindow * This,
            /* [out] */ LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *LoadState )( 
            IPluginWindow * This,
            /* [in] */ VARIANT_BOOL bMaximise);
        
        HRESULT ( STDMETHODCALLTYPE *SaveState )( 
            IPluginWindow * This);
        
        HRESULT ( STDMETHODCALLTYPE *ThrowMenu )( 
            IPluginWindow * This,
            /* [in] */ BSTR sName,
            /* [in] */ LONG nDefaultID,
            /* [in] */ POINT *pPoint);
        
        HRESULT ( STDMETHODCALLTYPE *AddToolbar )( 
            IPluginWindow * This,
            /* [in] */ BSTR sName,
            /* [in] */ LONG nPosition,
            /* [out] */ HWND *phWnd,
            /* [retval][out] */ ISToolbar **ppToolbar);
        
        HRESULT ( STDMETHODCALLTYPE *AdjustWindowRect )( 
            IPluginWindow * This,
            /* [out][in] */ RECT *pRect,
            /* [in] */ VARIANT_BOOL bClientToWindow);
        
        END_INTERFACE
    } IPluginWindowVtbl;

    interface IPluginWindow
    {
        CONST_VTBL struct IPluginWindowVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPluginWindow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPluginWindow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPluginWindow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPluginWindow_ListenForSingleMessage(This,nMessage)	\
    (This)->lpVtbl -> ListenForSingleMessage(This,nMessage)

#define IPluginWindow_ListenForMultipleMessages(This,pMessages)	\
    (This)->lpVtbl -> ListenForMultipleMessages(This,pMessages)

#define IPluginWindow_Create1(This,bsCaption,hIcon,bPanel,bTabbed)	\
    (This)->lpVtbl -> Create1(This,bsCaption,hIcon,bPanel,bTabbed)

#define IPluginWindow_Create2(This,nCommandID,bPanel,bTabbed)	\
    (This)->lpVtbl -> Create2(This,nCommandID,bPanel,bTabbed)

#define IPluginWindow_GetHwnd(This,phWnd)	\
    (This)->lpVtbl -> GetHwnd(This,phWnd)

#define IPluginWindow_HandleMessage(This,plResult)	\
    (This)->lpVtbl -> HandleMessage(This,plResult)

#define IPluginWindow_LoadState(This,bMaximise)	\
    (This)->lpVtbl -> LoadState(This,bMaximise)

#define IPluginWindow_SaveState(This)	\
    (This)->lpVtbl -> SaveState(This)

#define IPluginWindow_ThrowMenu(This,sName,nDefaultID,pPoint)	\
    (This)->lpVtbl -> ThrowMenu(This,sName,nDefaultID,pPoint)

#define IPluginWindow_AddToolbar(This,sName,nPosition,phWnd,ppToolbar)	\
    (This)->lpVtbl -> AddToolbar(This,sName,nPosition,phWnd,ppToolbar)

#define IPluginWindow_AdjustWindowRect(This,pRect,bClientToWindow)	\
    (This)->lpVtbl -> AdjustWindowRect(This,pRect,bClientToWindow)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPluginWindow_ListenForSingleMessage_Proxy( 
    IPluginWindow * This,
    /* [in] */ UINT nMessage);


void __RPC_STUB IPluginWindow_ListenForSingleMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_ListenForMultipleMessages_Proxy( 
    IPluginWindow * This,
    /* [in] */ SAFEARRAY *pMessages);


void __RPC_STUB IPluginWindow_ListenForMultipleMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_Create1_Proxy( 
    IPluginWindow * This,
    /* [in] */ BSTR bsCaption,
    /* [in] */ HICON hIcon,
    /* [in] */ VARIANT_BOOL bPanel,
    /* [in] */ VARIANT_BOOL bTabbed);


void __RPC_STUB IPluginWindow_Create1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_Create2_Proxy( 
    IPluginWindow * This,
    /* [in] */ UINT nCommandID,
    /* [in] */ VARIANT_BOOL bPanel,
    /* [in] */ VARIANT_BOOL bTabbed);


void __RPC_STUB IPluginWindow_Create2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_GetHwnd_Proxy( 
    IPluginWindow * This,
    /* [out] */ HWND *phWnd);


void __RPC_STUB IPluginWindow_GetHwnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_HandleMessage_Proxy( 
    IPluginWindow * This,
    /* [out] */ LRESULT *plResult);


void __RPC_STUB IPluginWindow_HandleMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_LoadState_Proxy( 
    IPluginWindow * This,
    /* [in] */ VARIANT_BOOL bMaximise);


void __RPC_STUB IPluginWindow_LoadState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_SaveState_Proxy( 
    IPluginWindow * This);


void __RPC_STUB IPluginWindow_SaveState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_ThrowMenu_Proxy( 
    IPluginWindow * This,
    /* [in] */ BSTR sName,
    /* [in] */ LONG nDefaultID,
    /* [in] */ POINT *pPoint);


void __RPC_STUB IPluginWindow_ThrowMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_AddToolbar_Proxy( 
    IPluginWindow * This,
    /* [in] */ BSTR sName,
    /* [in] */ LONG nPosition,
    /* [out] */ HWND *phWnd,
    /* [retval][out] */ ISToolbar **ppToolbar);


void __RPC_STUB IPluginWindow_AddToolbar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindow_AdjustWindowRect_Proxy( 
    IPluginWindow * This,
    /* [out][in] */ RECT *pRect,
    /* [in] */ VARIANT_BOOL bClientToWindow);


void __RPC_STUB IPluginWindow_AdjustWindowRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPluginWindow_INTERFACE_DEFINED__ */


#ifndef __IPluginWindowOwner_INTERFACE_DEFINED__
#define __IPluginWindowOwner_INTERFACE_DEFINED__

/* interface IPluginWindowOwner */
/* [object][uuid] */ 


DEFINE_GUID(IID_IPluginWindowOwner,0xC6631461,0x2654,0x4572,0xBB,0x3C,0x54,0xF5,0x2F,0x0F,0xF1,0xB9);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C6631461-2654-4572-BB3C-54F52F0FF1B9")
    IPluginWindowOwner : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnTranslate( 
            /* [in] */ MSG *pMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMessage( 
            /* [in] */ UINT nMessage,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam,
            /* [out] */ LRESULT *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnUpdate( 
            /* [in] */ UINT nCommandID,
            /* [out][in] */ STRISTATE *pbVisible,
            /* [out][in] */ STRISTATE *pbEnabled,
            /* [out][in] */ STRISTATE *pbChecked) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCommand( 
            /* [in] */ UINT nCommandID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPluginWindowOwnerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPluginWindowOwner * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPluginWindowOwner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPluginWindowOwner * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnTranslate )( 
            IPluginWindowOwner * This,
            /* [in] */ MSG *pMessage);
        
        HRESULT ( STDMETHODCALLTYPE *OnMessage )( 
            IPluginWindowOwner * This,
            /* [in] */ UINT nMessage,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam,
            /* [out] */ LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *OnUpdate )( 
            IPluginWindowOwner * This,
            /* [in] */ UINT nCommandID,
            /* [out][in] */ STRISTATE *pbVisible,
            /* [out][in] */ STRISTATE *pbEnabled,
            /* [out][in] */ STRISTATE *pbChecked);
        
        HRESULT ( STDMETHODCALLTYPE *OnCommand )( 
            IPluginWindowOwner * This,
            /* [in] */ UINT nCommandID);
        
        END_INTERFACE
    } IPluginWindowOwnerVtbl;

    interface IPluginWindowOwner
    {
        CONST_VTBL struct IPluginWindowOwnerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPluginWindowOwner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPluginWindowOwner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPluginWindowOwner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPluginWindowOwner_OnTranslate(This,pMessage)	\
    (This)->lpVtbl -> OnTranslate(This,pMessage)

#define IPluginWindowOwner_OnMessage(This,nMessage,wParam,lParam,plResult)	\
    (This)->lpVtbl -> OnMessage(This,nMessage,wParam,lParam,plResult)

#define IPluginWindowOwner_OnUpdate(This,nCommandID,pbVisible,pbEnabled,pbChecked)	\
    (This)->lpVtbl -> OnUpdate(This,nCommandID,pbVisible,pbEnabled,pbChecked)

#define IPluginWindowOwner_OnCommand(This,nCommandID)	\
    (This)->lpVtbl -> OnCommand(This,nCommandID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPluginWindowOwner_OnTranslate_Proxy( 
    IPluginWindowOwner * This,
    /* [in] */ MSG *pMessage);


void __RPC_STUB IPluginWindowOwner_OnTranslate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindowOwner_OnMessage_Proxy( 
    IPluginWindowOwner * This,
    /* [in] */ UINT nMessage,
    /* [in] */ WPARAM wParam,
    /* [in] */ LPARAM lParam,
    /* [out] */ LRESULT *plResult);


void __RPC_STUB IPluginWindowOwner_OnMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindowOwner_OnUpdate_Proxy( 
    IPluginWindowOwner * This,
    /* [in] */ UINT nCommandID,
    /* [out][in] */ STRISTATE *pbVisible,
    /* [out][in] */ STRISTATE *pbEnabled,
    /* [out][in] */ STRISTATE *pbChecked);


void __RPC_STUB IPluginWindowOwner_OnUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPluginWindowOwner_OnCommand_Proxy( 
    IPluginWindowOwner * This,
    /* [in] */ UINT nCommandID);


void __RPC_STUB IPluginWindowOwner_OnCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPluginWindowOwner_INTERFACE_DEFINED__ */


#ifndef __ILibraryBuilderPlugin_INTERFACE_DEFINED__
#define __ILibraryBuilderPlugin_INTERFACE_DEFINED__

/* interface ILibraryBuilderPlugin */
/* [object][uuid] */ 


DEFINE_GUID(IID_ILibraryBuilderPlugin,0x32496CEA,0x3B51,0x4f2f,0x9C,0xE7,0x73,0xD6,0xAC,0x94,0x2C,0x34);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("32496CEA-3B51-4f2f-9CE7-73D6AC942C34")
    ILibraryBuilderPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Process( 
            /* [in] */ HANDLE hFile,
            /* [in] */ BSTR sFile,
            /* [in] */ ISXMLElement *pXML) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILibraryBuilderPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILibraryBuilderPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILibraryBuilderPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILibraryBuilderPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *Process )( 
            ILibraryBuilderPlugin * This,
            /* [in] */ HANDLE hFile,
            /* [in] */ BSTR sFile,
            /* [in] */ ISXMLElement *pXML);
        
        END_INTERFACE
    } ILibraryBuilderPluginVtbl;

    interface ILibraryBuilderPlugin
    {
        CONST_VTBL struct ILibraryBuilderPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILibraryBuilderPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILibraryBuilderPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILibraryBuilderPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILibraryBuilderPlugin_Process(This,hFile,sFile,pXML)	\
    (This)->lpVtbl -> Process(This,hFile,sFile,pXML)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ILibraryBuilderPlugin_Process_Proxy( 
    ILibraryBuilderPlugin * This,
    /* [in] */ HANDLE hFile,
    /* [in] */ BSTR sFile,
    /* [in] */ ISXMLElement *pXML);


void __RPC_STUB ILibraryBuilderPlugin_Process_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILibraryBuilderPlugin_INTERFACE_DEFINED__ */


#ifndef __IImageServicePlugin_INTERFACE_DEFINED__
#define __IImageServicePlugin_INTERFACE_DEFINED__

/* interface IImageServicePlugin */
/* [object][uuid] */ 


DEFINE_GUID(IID_IImageServicePlugin,0xAD49786E,0x09C1,0x4069,0xB4,0x8A,0x15,0xAD,0x42,0xD6,0xCE,0xD1);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD49786E-09C1-4069-B48A-15AD42D6CED1")
    IImageServicePlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LoadFromFile( 
            /* [in] */ HANDLE hFile,
            /* [in] */ DWORD nLength,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY **ppImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadFromMemory( 
            /* [in] */ SAFEARRAY *pMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY **ppImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveToFile( 
            /* [in] */ HANDLE hFile,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY *pImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveToMemory( 
            /* [out] */ SAFEARRAY **ppMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY *pImage) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImageServicePluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IImageServicePlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IImageServicePlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IImageServicePlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *LoadFromFile )( 
            IImageServicePlugin * This,
            /* [in] */ HANDLE hFile,
            /* [in] */ DWORD nLength,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY **ppImage);
        
        HRESULT ( STDMETHODCALLTYPE *LoadFromMemory )( 
            IImageServicePlugin * This,
            /* [in] */ SAFEARRAY *pMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [out] */ SAFEARRAY **ppImage);
        
        HRESULT ( STDMETHODCALLTYPE *SaveToFile )( 
            IImageServicePlugin * This,
            /* [in] */ HANDLE hFile,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY *pImage);
        
        HRESULT ( STDMETHODCALLTYPE *SaveToMemory )( 
            IImageServicePlugin * This,
            /* [out] */ SAFEARRAY **ppMemory,
            /* [out][in] */ IMAGESERVICEDATA *pParams,
            /* [in] */ SAFEARRAY *pImage);
        
        END_INTERFACE
    } IImageServicePluginVtbl;

    interface IImageServicePlugin
    {
        CONST_VTBL struct IImageServicePluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImageServicePlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImageServicePlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImageServicePlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImageServicePlugin_LoadFromFile(This,hFile,nLength,pParams,ppImage)	\
    (This)->lpVtbl -> LoadFromFile(This,hFile,nLength,pParams,ppImage)

#define IImageServicePlugin_LoadFromMemory(This,pMemory,pParams,ppImage)	\
    (This)->lpVtbl -> LoadFromMemory(This,pMemory,pParams,ppImage)

#define IImageServicePlugin_SaveToFile(This,hFile,pParams,pImage)	\
    (This)->lpVtbl -> SaveToFile(This,hFile,pParams,pImage)

#define IImageServicePlugin_SaveToMemory(This,ppMemory,pParams,pImage)	\
    (This)->lpVtbl -> SaveToMemory(This,ppMemory,pParams,pImage)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IImageServicePlugin_LoadFromFile_Proxy( 
    IImageServicePlugin * This,
    /* [in] */ HANDLE hFile,
    /* [in] */ DWORD nLength,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [out] */ SAFEARRAY **ppImage);


void __RPC_STUB IImageServicePlugin_LoadFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImageServicePlugin_LoadFromMemory_Proxy( 
    IImageServicePlugin * This,
    /* [in] */ SAFEARRAY *pMemory,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [out] */ SAFEARRAY **ppImage);


void __RPC_STUB IImageServicePlugin_LoadFromMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImageServicePlugin_SaveToFile_Proxy( 
    IImageServicePlugin * This,
    /* [in] */ HANDLE hFile,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [in] */ SAFEARRAY *pImage);


void __RPC_STUB IImageServicePlugin_SaveToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImageServicePlugin_SaveToMemory_Proxy( 
    IImageServicePlugin * This,
    /* [out] */ SAFEARRAY **ppMemory,
    /* [out][in] */ IMAGESERVICEDATA *pParams,
    /* [in] */ SAFEARRAY *pImage);


void __RPC_STUB IImageServicePlugin_SaveToMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IImageServicePlugin_INTERFACE_DEFINED__ */


#ifndef __IDownloadPreviewSite_INTERFACE_DEFINED__
#define __IDownloadPreviewSite_INTERFACE_DEFINED__

/* interface IDownloadPreviewSite */
/* [object][uuid] */ 


DEFINE_GUID(IID_IDownloadPreviewSite,0x52A97CBD,0x2B99,0x45e8,0xB6,0xF9,0x41,0xE9,0xCD,0x58,0x39,0x60);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("52A97CBD-2B99-45e8-B6F9-41E9CD583960")
    IDownloadPreviewSite : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSuggestedFilename( 
            /* [retval][out] */ BSTR *psFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAvailableRanges( 
            /* [retval][out] */ SAFEARRAY **pArray) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProgressRange( 
            /* [in] */ DWORD nRange) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProgressPosition( 
            /* [in] */ DWORD nPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProgressMessage( 
            /* [in] */ BSTR sMessage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueueDeleteFile( 
            /* [in] */ BSTR sTempFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExecuteFile( 
            /* [in] */ BSTR sFile) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDownloadPreviewSiteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDownloadPreviewSite * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDownloadPreviewSite * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDownloadPreviewSite * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSuggestedFilename )( 
            IDownloadPreviewSite * This,
            /* [retval][out] */ BSTR *psFile);
        
        HRESULT ( STDMETHODCALLTYPE *GetAvailableRanges )( 
            IDownloadPreviewSite * This,
            /* [retval][out] */ SAFEARRAY **pArray);
        
        HRESULT ( STDMETHODCALLTYPE *SetProgressRange )( 
            IDownloadPreviewSite * This,
            /* [in] */ DWORD nRange);
        
        HRESULT ( STDMETHODCALLTYPE *SetProgressPosition )( 
            IDownloadPreviewSite * This,
            /* [in] */ DWORD nPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetProgressMessage )( 
            IDownloadPreviewSite * This,
            /* [in] */ BSTR sMessage);
        
        HRESULT ( STDMETHODCALLTYPE *QueueDeleteFile )( 
            IDownloadPreviewSite * This,
            /* [in] */ BSTR sTempFile);
        
        HRESULT ( STDMETHODCALLTYPE *ExecuteFile )( 
            IDownloadPreviewSite * This,
            /* [in] */ BSTR sFile);
        
        END_INTERFACE
    } IDownloadPreviewSiteVtbl;

    interface IDownloadPreviewSite
    {
        CONST_VTBL struct IDownloadPreviewSiteVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDownloadPreviewSite_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDownloadPreviewSite_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDownloadPreviewSite_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDownloadPreviewSite_GetSuggestedFilename(This,psFile)	\
    (This)->lpVtbl -> GetSuggestedFilename(This,psFile)

#define IDownloadPreviewSite_GetAvailableRanges(This,pArray)	\
    (This)->lpVtbl -> GetAvailableRanges(This,pArray)

#define IDownloadPreviewSite_SetProgressRange(This,nRange)	\
    (This)->lpVtbl -> SetProgressRange(This,nRange)

#define IDownloadPreviewSite_SetProgressPosition(This,nPosition)	\
    (This)->lpVtbl -> SetProgressPosition(This,nPosition)

#define IDownloadPreviewSite_SetProgressMessage(This,sMessage)	\
    (This)->lpVtbl -> SetProgressMessage(This,sMessage)

#define IDownloadPreviewSite_QueueDeleteFile(This,sTempFile)	\
    (This)->lpVtbl -> QueueDeleteFile(This,sTempFile)

#define IDownloadPreviewSite_ExecuteFile(This,sFile)	\
    (This)->lpVtbl -> ExecuteFile(This,sFile)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDownloadPreviewSite_GetSuggestedFilename_Proxy( 
    IDownloadPreviewSite * This,
    /* [retval][out] */ BSTR *psFile);


void __RPC_STUB IDownloadPreviewSite_GetSuggestedFilename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewSite_GetAvailableRanges_Proxy( 
    IDownloadPreviewSite * This,
    /* [retval][out] */ SAFEARRAY **pArray);


void __RPC_STUB IDownloadPreviewSite_GetAvailableRanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewSite_SetProgressRange_Proxy( 
    IDownloadPreviewSite * This,
    /* [in] */ DWORD nRange);


void __RPC_STUB IDownloadPreviewSite_SetProgressRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewSite_SetProgressPosition_Proxy( 
    IDownloadPreviewSite * This,
    /* [in] */ DWORD nPosition);


void __RPC_STUB IDownloadPreviewSite_SetProgressPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewSite_SetProgressMessage_Proxy( 
    IDownloadPreviewSite * This,
    /* [in] */ BSTR sMessage);


void __RPC_STUB IDownloadPreviewSite_SetProgressMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewSite_QueueDeleteFile_Proxy( 
    IDownloadPreviewSite * This,
    /* [in] */ BSTR sTempFile);


void __RPC_STUB IDownloadPreviewSite_QueueDeleteFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewSite_ExecuteFile_Proxy( 
    IDownloadPreviewSite * This,
    /* [in] */ BSTR sFile);


void __RPC_STUB IDownloadPreviewSite_ExecuteFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDownloadPreviewSite_INTERFACE_DEFINED__ */


#ifndef __IDownloadPreviewPlugin_INTERFACE_DEFINED__
#define __IDownloadPreviewPlugin_INTERFACE_DEFINED__

/* interface IDownloadPreviewPlugin */
/* [object][uuid] */ 


DEFINE_GUID(IID_IDownloadPreviewPlugin,0x1182FCD9,0x9F14,0x4e4a,0xBD,0x05,0x43,0x24,0x22,0xB5,0xBF,0xAF);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1182FCD9-9F14-4e4a-BD05-432422B5BFAF")
    IDownloadPreviewPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSite( 
            /* [in] */ IDownloadPreviewSite *pSite) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Preview( 
            /* [in] */ HANDLE hFile,
            /* [in] */ BSTR sFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDownloadPreviewPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDownloadPreviewPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDownloadPreviewPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDownloadPreviewPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSite )( 
            IDownloadPreviewPlugin * This,
            /* [in] */ IDownloadPreviewSite *pSite);
        
        HRESULT ( STDMETHODCALLTYPE *Preview )( 
            IDownloadPreviewPlugin * This,
            /* [in] */ HANDLE hFile,
            /* [in] */ BSTR sFile);
        
        HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IDownloadPreviewPlugin * This);
        
        END_INTERFACE
    } IDownloadPreviewPluginVtbl;

    interface IDownloadPreviewPlugin
    {
        CONST_VTBL struct IDownloadPreviewPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDownloadPreviewPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDownloadPreviewPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDownloadPreviewPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDownloadPreviewPlugin_SetSite(This,pSite)	\
    (This)->lpVtbl -> SetSite(This,pSite)

#define IDownloadPreviewPlugin_Preview(This,hFile,sFile)	\
    (This)->lpVtbl -> Preview(This,hFile,sFile)

#define IDownloadPreviewPlugin_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDownloadPreviewPlugin_SetSite_Proxy( 
    IDownloadPreviewPlugin * This,
    /* [in] */ IDownloadPreviewSite *pSite);


void __RPC_STUB IDownloadPreviewPlugin_SetSite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewPlugin_Preview_Proxy( 
    IDownloadPreviewPlugin * This,
    /* [in] */ HANDLE hFile,
    /* [in] */ BSTR sFile);


void __RPC_STUB IDownloadPreviewPlugin_Preview_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDownloadPreviewPlugin_Cancel_Proxy( 
    IDownloadPreviewPlugin * This);


void __RPC_STUB IDownloadPreviewPlugin_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDownloadPreviewPlugin_INTERFACE_DEFINED__ */


#ifndef __IAudioVisPlugin_INTERFACE_DEFINED__
#define __IAudioVisPlugin_INTERFACE_DEFINED__

/* interface IAudioVisPlugin */
/* [object][uuid] */ 


DEFINE_GUID(IID_IAudioVisPlugin,0xE788D125,0x4D41,0x4a35,0xA4,0x36,0xD1,0xA1,0xFD,0x0C,0x8E,0xC9);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E788D125-4D41-4a35-A436-D1A1FD0C8EC9")
    IAudioVisPlugin : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Configure( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ HWND hWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reposition( 
            /* [in] */ RECT *prcWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTrackOpen( 
            /* [in] */ BSTR sName,
            /* [in] */ DWORD nLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTrackClose( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPlayStart( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnPlayStop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnChunk( 
            /* [in] */ SHAREAZAVISCHUNK *pChunk) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAudioVisPluginVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAudioVisPlugin * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAudioVisPlugin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAudioVisPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *Configure )( 
            IAudioVisPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IAudioVisPlugin * This,
            /* [in] */ HWND hWnd);
        
        HRESULT ( STDMETHODCALLTYPE *Destroy )( 
            IAudioVisPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reposition )( 
            IAudioVisPlugin * This,
            /* [in] */ RECT *prcWnd);
        
        HRESULT ( STDMETHODCALLTYPE *OnTrackOpen )( 
            IAudioVisPlugin * This,
            /* [in] */ BSTR sName,
            /* [in] */ DWORD nLength);
        
        HRESULT ( STDMETHODCALLTYPE *OnTrackClose )( 
            IAudioVisPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPlayStart )( 
            IAudioVisPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnPlayStop )( 
            IAudioVisPlugin * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnChunk )( 
            IAudioVisPlugin * This,
            /* [in] */ SHAREAZAVISCHUNK *pChunk);
        
        END_INTERFACE
    } IAudioVisPluginVtbl;

    interface IAudioVisPlugin
    {
        CONST_VTBL struct IAudioVisPluginVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAudioVisPlugin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAudioVisPlugin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAudioVisPlugin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAudioVisPlugin_Configure(This)	\
    (This)->lpVtbl -> Configure(This)

#define IAudioVisPlugin_Create(This,hWnd)	\
    (This)->lpVtbl -> Create(This,hWnd)

#define IAudioVisPlugin_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#define IAudioVisPlugin_Reposition(This,prcWnd)	\
    (This)->lpVtbl -> Reposition(This,prcWnd)

#define IAudioVisPlugin_OnTrackOpen(This,sName,nLength)	\
    (This)->lpVtbl -> OnTrackOpen(This,sName,nLength)

#define IAudioVisPlugin_OnTrackClose(This)	\
    (This)->lpVtbl -> OnTrackClose(This)

#define IAudioVisPlugin_OnPlayStart(This)	\
    (This)->lpVtbl -> OnPlayStart(This)

#define IAudioVisPlugin_OnPlayStop(This)	\
    (This)->lpVtbl -> OnPlayStop(This)

#define IAudioVisPlugin_OnChunk(This,pChunk)	\
    (This)->lpVtbl -> OnChunk(This,pChunk)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAudioVisPlugin_Configure_Proxy( 
    IAudioVisPlugin * This);


void __RPC_STUB IAudioVisPlugin_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_Create_Proxy( 
    IAudioVisPlugin * This,
    /* [in] */ HWND hWnd);


void __RPC_STUB IAudioVisPlugin_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_Destroy_Proxy( 
    IAudioVisPlugin * This);


void __RPC_STUB IAudioVisPlugin_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_Reposition_Proxy( 
    IAudioVisPlugin * This,
    /* [in] */ RECT *prcWnd);


void __RPC_STUB IAudioVisPlugin_Reposition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_OnTrackOpen_Proxy( 
    IAudioVisPlugin * This,
    /* [in] */ BSTR sName,
    /* [in] */ DWORD nLength);


void __RPC_STUB IAudioVisPlugin_OnTrackOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_OnTrackClose_Proxy( 
    IAudioVisPlugin * This);


void __RPC_STUB IAudioVisPlugin_OnTrackClose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_OnPlayStart_Proxy( 
    IAudioVisPlugin * This);


void __RPC_STUB IAudioVisPlugin_OnPlayStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_OnPlayStop_Proxy( 
    IAudioVisPlugin * This);


void __RPC_STUB IAudioVisPlugin_OnPlayStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAudioVisPlugin_OnChunk_Proxy( 
    IAudioVisPlugin * This,
    /* [in] */ SHAREAZAVISCHUNK *pChunk);


void __RPC_STUB IAudioVisPlugin_OnChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAudioVisPlugin_INTERFACE_DEFINED__ */


#ifndef __IMediaPlayer_INTERFACE_DEFINED__
#define __IMediaPlayer_INTERFACE_DEFINED__

/* interface IMediaPlayer */
/* [object][unique][uuid] */ 


DEFINE_GUID(IID_IMediaPlayer,0x59978299,0xC8AC,0x4818,0x83,0xF4,0xC3,0x82,0xBB,0x61,0x1D,0x5C);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59978299-C8AC-4818-83F4-C382BB611D5C")
    IMediaPlayer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ HWND hWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reposition( 
            /* [in] */ RECT *prcWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLogoBitmap( 
            /* [in] */ HBITMAP hLogo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVolume( 
            /* [out] */ DOUBLE *pnVolume) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVolume( 
            /* [in] */ DOUBLE nVolume) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetZoom( 
            /* [out] */ MediaZoom *pnZoom) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetZoom( 
            /* [in] */ MediaZoom nZoom) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAspect( 
            /* [out] */ DOUBLE *pnAspect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAspect( 
            /* [in] */ DOUBLE nAspect) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ BSTR sFilename) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Play( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetState( 
            /* [out] */ MediaState *pnState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLength( 
            /* [out] */ LONGLONG *pnLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPosition( 
            /* [out] */ LONGLONG *pnPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPosition( 
            /* [in] */ LONGLONG nPosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSpeed( 
            /* [out] */ DOUBLE *pnSpeed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSpeed( 
            /* [in] */ DOUBLE nSpeed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPlugin( 
            /* [out] */ IAudioVisPlugin **ppPlugin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPlugin( 
            /* [in] */ IAudioVisPlugin *pPlugin) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPluginSize( 
            /* [out] */ LONG *pnSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPluginSize( 
            /* [in] */ LONG nSize) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMediaPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMediaPlayer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMediaPlayer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMediaPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IMediaPlayer * This,
            /* [in] */ HWND hWnd);
        
        HRESULT ( STDMETHODCALLTYPE *Destroy )( 
            IMediaPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Reposition )( 
            IMediaPlayer * This,
            /* [in] */ RECT *prcWnd);
        
        HRESULT ( STDMETHODCALLTYPE *SetLogoBitmap )( 
            IMediaPlayer * This,
            /* [in] */ HBITMAP hLogo);
        
        HRESULT ( STDMETHODCALLTYPE *GetVolume )( 
            IMediaPlayer * This,
            /* [out] */ DOUBLE *pnVolume);
        
        HRESULT ( STDMETHODCALLTYPE *SetVolume )( 
            IMediaPlayer * This,
            /* [in] */ DOUBLE nVolume);
        
        HRESULT ( STDMETHODCALLTYPE *GetZoom )( 
            IMediaPlayer * This,
            /* [out] */ MediaZoom *pnZoom);
        
        HRESULT ( STDMETHODCALLTYPE *SetZoom )( 
            IMediaPlayer * This,
            /* [in] */ MediaZoom nZoom);
        
        HRESULT ( STDMETHODCALLTYPE *GetAspect )( 
            IMediaPlayer * This,
            /* [out] */ DOUBLE *pnAspect);
        
        HRESULT ( STDMETHODCALLTYPE *SetAspect )( 
            IMediaPlayer * This,
            /* [in] */ DOUBLE nAspect);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IMediaPlayer * This,
            /* [in] */ BSTR sFilename);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IMediaPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Play )( 
            IMediaPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMediaPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMediaPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetState )( 
            IMediaPlayer * This,
            /* [out] */ MediaState *pnState);
        
        HRESULT ( STDMETHODCALLTYPE *GetLength )( 
            IMediaPlayer * This,
            /* [out] */ LONGLONG *pnLength);
        
        HRESULT ( STDMETHODCALLTYPE *GetPosition )( 
            IMediaPlayer * This,
            /* [out] */ LONGLONG *pnPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetPosition )( 
            IMediaPlayer * This,
            /* [in] */ LONGLONG nPosition);
        
        HRESULT ( STDMETHODCALLTYPE *GetSpeed )( 
            IMediaPlayer * This,
            /* [out] */ DOUBLE *pnSpeed);
        
        HRESULT ( STDMETHODCALLTYPE *SetSpeed )( 
            IMediaPlayer * This,
            /* [in] */ DOUBLE nSpeed);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlugin )( 
            IMediaPlayer * This,
            /* [out] */ IAudioVisPlugin **ppPlugin);
        
        HRESULT ( STDMETHODCALLTYPE *SetPlugin )( 
            IMediaPlayer * This,
            /* [in] */ IAudioVisPlugin *pPlugin);
        
        HRESULT ( STDMETHODCALLTYPE *GetPluginSize )( 
            IMediaPlayer * This,
            /* [out] */ LONG *pnSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetPluginSize )( 
            IMediaPlayer * This,
            /* [in] */ LONG nSize);
        
        END_INTERFACE
    } IMediaPlayerVtbl;

    interface IMediaPlayer
    {
        CONST_VTBL struct IMediaPlayerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMediaPlayer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMediaPlayer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMediaPlayer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMediaPlayer_Create(This,hWnd)	\
    (This)->lpVtbl -> Create(This,hWnd)

#define IMediaPlayer_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#define IMediaPlayer_Reposition(This,prcWnd)	\
    (This)->lpVtbl -> Reposition(This,prcWnd)

#define IMediaPlayer_SetLogoBitmap(This,hLogo)	\
    (This)->lpVtbl -> SetLogoBitmap(This,hLogo)

#define IMediaPlayer_GetVolume(This,pnVolume)	\
    (This)->lpVtbl -> GetVolume(This,pnVolume)

#define IMediaPlayer_SetVolume(This,nVolume)	\
    (This)->lpVtbl -> SetVolume(This,nVolume)

#define IMediaPlayer_GetZoom(This,pnZoom)	\
    (This)->lpVtbl -> GetZoom(This,pnZoom)

#define IMediaPlayer_SetZoom(This,nZoom)	\
    (This)->lpVtbl -> SetZoom(This,nZoom)

#define IMediaPlayer_GetAspect(This,pnAspect)	\
    (This)->lpVtbl -> GetAspect(This,pnAspect)

#define IMediaPlayer_SetAspect(This,nAspect)	\
    (This)->lpVtbl -> SetAspect(This,nAspect)

#define IMediaPlayer_Open(This,sFilename)	\
    (This)->lpVtbl -> Open(This,sFilename)

#define IMediaPlayer_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IMediaPlayer_Play(This)	\
    (This)->lpVtbl -> Play(This)

#define IMediaPlayer_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMediaPlayer_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMediaPlayer_GetState(This,pnState)	\
    (This)->lpVtbl -> GetState(This,pnState)

#define IMediaPlayer_GetLength(This,pnLength)	\
    (This)->lpVtbl -> GetLength(This,pnLength)

#define IMediaPlayer_GetPosition(This,pnPosition)	\
    (This)->lpVtbl -> GetPosition(This,pnPosition)

#define IMediaPlayer_SetPosition(This,nPosition)	\
    (This)->lpVtbl -> SetPosition(This,nPosition)

#define IMediaPlayer_GetSpeed(This,pnSpeed)	\
    (This)->lpVtbl -> GetSpeed(This,pnSpeed)

#define IMediaPlayer_SetSpeed(This,nSpeed)	\
    (This)->lpVtbl -> SetSpeed(This,nSpeed)

#define IMediaPlayer_GetPlugin(This,ppPlugin)	\
    (This)->lpVtbl -> GetPlugin(This,ppPlugin)

#define IMediaPlayer_SetPlugin(This,pPlugin)	\
    (This)->lpVtbl -> SetPlugin(This,pPlugin)

#define IMediaPlayer_GetPluginSize(This,pnSize)	\
    (This)->lpVtbl -> GetPluginSize(This,pnSize)

#define IMediaPlayer_SetPluginSize(This,nSize)	\
    (This)->lpVtbl -> SetPluginSize(This,nSize)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMediaPlayer_Create_Proxy( 
    IMediaPlayer * This,
    /* [in] */ HWND hWnd);


void __RPC_STUB IMediaPlayer_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_Destroy_Proxy( 
    IMediaPlayer * This);


void __RPC_STUB IMediaPlayer_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_Reposition_Proxy( 
    IMediaPlayer * This,
    /* [in] */ RECT *prcWnd);


void __RPC_STUB IMediaPlayer_Reposition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetLogoBitmap_Proxy( 
    IMediaPlayer * This,
    /* [in] */ HBITMAP hLogo);


void __RPC_STUB IMediaPlayer_SetLogoBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetVolume_Proxy( 
    IMediaPlayer * This,
    /* [out] */ DOUBLE *pnVolume);


void __RPC_STUB IMediaPlayer_GetVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetVolume_Proxy( 
    IMediaPlayer * This,
    /* [in] */ DOUBLE nVolume);


void __RPC_STUB IMediaPlayer_SetVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetZoom_Proxy( 
    IMediaPlayer * This,
    /* [out] */ MediaZoom *pnZoom);


void __RPC_STUB IMediaPlayer_GetZoom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetZoom_Proxy( 
    IMediaPlayer * This,
    /* [in] */ MediaZoom nZoom);


void __RPC_STUB IMediaPlayer_SetZoom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetAspect_Proxy( 
    IMediaPlayer * This,
    /* [out] */ DOUBLE *pnAspect);


void __RPC_STUB IMediaPlayer_GetAspect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetAspect_Proxy( 
    IMediaPlayer * This,
    /* [in] */ DOUBLE nAspect);


void __RPC_STUB IMediaPlayer_SetAspect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_Open_Proxy( 
    IMediaPlayer * This,
    /* [in] */ BSTR sFilename);


void __RPC_STUB IMediaPlayer_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_Close_Proxy( 
    IMediaPlayer * This);


void __RPC_STUB IMediaPlayer_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_Play_Proxy( 
    IMediaPlayer * This);


void __RPC_STUB IMediaPlayer_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_Pause_Proxy( 
    IMediaPlayer * This);


void __RPC_STUB IMediaPlayer_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_Stop_Proxy( 
    IMediaPlayer * This);


void __RPC_STUB IMediaPlayer_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetState_Proxy( 
    IMediaPlayer * This,
    /* [out] */ MediaState *pnState);


void __RPC_STUB IMediaPlayer_GetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetLength_Proxy( 
    IMediaPlayer * This,
    /* [out] */ LONGLONG *pnLength);


void __RPC_STUB IMediaPlayer_GetLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetPosition_Proxy( 
    IMediaPlayer * This,
    /* [out] */ LONGLONG *pnPosition);


void __RPC_STUB IMediaPlayer_GetPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetPosition_Proxy( 
    IMediaPlayer * This,
    /* [in] */ LONGLONG nPosition);


void __RPC_STUB IMediaPlayer_SetPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetSpeed_Proxy( 
    IMediaPlayer * This,
    /* [out] */ DOUBLE *pnSpeed);


void __RPC_STUB IMediaPlayer_GetSpeed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetSpeed_Proxy( 
    IMediaPlayer * This,
    /* [in] */ DOUBLE nSpeed);


void __RPC_STUB IMediaPlayer_SetSpeed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetPlugin_Proxy( 
    IMediaPlayer * This,
    /* [out] */ IAudioVisPlugin **ppPlugin);


void __RPC_STUB IMediaPlayer_GetPlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetPlugin_Proxy( 
    IMediaPlayer * This,
    /* [in] */ IAudioVisPlugin *pPlugin);


void __RPC_STUB IMediaPlayer_SetPlugin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_GetPluginSize_Proxy( 
    IMediaPlayer * This,
    /* [out] */ LONG *pnSize);


void __RPC_STUB IMediaPlayer_GetPluginSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMediaPlayer_SetPluginSize_Proxy( 
    IMediaPlayer * This,
    /* [in] */ LONG nSize);


void __RPC_STUB IMediaPlayer_SetPluginSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMediaPlayer_INTERFACE_DEFINED__ */


#ifndef __IWrappedPluginControl_INTERFACE_DEFINED__
#define __IWrappedPluginControl_INTERFACE_DEFINED__

/* interface IWrappedPluginControl */
/* [object][uuid] */ 


DEFINE_GUID(IID_IWrappedPluginControl,0x71045028,0xF95C,0x4c03,0xAB,0x09,0x6D,0x90,0x6D,0xBF,0xC7,0x31);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("71045028-F95C-4c03-AB09-6D906DBFC731")
    IWrappedPluginControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ BSTR sDLL,
            /* [in] */ LONG nIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unload( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enumerate( 
            /* [out] */ LPSAFEARRAY *ppArray) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IWrappedPluginControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWrappedPluginControl * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWrappedPluginControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWrappedPluginControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Load )( 
            IWrappedPluginControl * This,
            /* [in] */ BSTR sDLL,
            /* [in] */ LONG nIndex);
        
        HRESULT ( STDMETHODCALLTYPE *Unload )( 
            IWrappedPluginControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Enumerate )( 
            IWrappedPluginControl * This,
            /* [out] */ LPSAFEARRAY *ppArray);
        
        END_INTERFACE
    } IWrappedPluginControlVtbl;

    interface IWrappedPluginControl
    {
        CONST_VTBL struct IWrappedPluginControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWrappedPluginControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWrappedPluginControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWrappedPluginControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWrappedPluginControl_Load(This,sDLL,nIndex)	\
    (This)->lpVtbl -> Load(This,sDLL,nIndex)

#define IWrappedPluginControl_Unload(This)	\
    (This)->lpVtbl -> Unload(This)

#define IWrappedPluginControl_Enumerate(This,ppArray)	\
    (This)->lpVtbl -> Enumerate(This,ppArray)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IWrappedPluginControl_Load_Proxy( 
    IWrappedPluginControl * This,
    /* [in] */ BSTR sDLL,
    /* [in] */ LONG nIndex);


void __RPC_STUB IWrappedPluginControl_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWrappedPluginControl_Unload_Proxy( 
    IWrappedPluginControl * This);


void __RPC_STUB IWrappedPluginControl_Unload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWrappedPluginControl_Enumerate_Proxy( 
    IWrappedPluginControl * This,
    /* [out] */ LPSAFEARRAY *ppArray);


void __RPC_STUB IWrappedPluginControl_Enumerate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IWrappedPluginControl_INTERFACE_DEFINED__ */


#ifndef __IUserInterface_INTERFACE_DEFINED__
#define __IUserInterface_INTERFACE_DEFINED__

/* interface IUserInterface */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_IUserInterface,0xFCDE733E,0xDDA0,0x4849,0xAD,0x83,0xD0,0x2B,0x0F,0x4D,0x1D,0xA3);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FCDE733E-DDA0-4849-AD83-D02B0F4D1DA3")
    IUserInterface : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UserInterface( 
            /* [retval][out] */ IUserInterface **ppUserInterface) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE NewWindow( 
            /* [in] */ BSTR bsName,
            /* [in] */ IPluginWindowOwner *pOwner,
            /* [retval][out] */ IPluginWindow **ppWindow) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_MainWindowHwnd( 
            /* [retval][out] */ HWND *phWnd) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ActiveView( 
            /* [retval][out] */ IGenericView **ppView) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RegisterCommand( 
            /* [in] */ BSTR bsName,
            /* [in] */ HICON hIcon,
            /* [retval][out] */ UINT *pnCommand) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddFromString( 
            /* [in] */ BSTR sXML) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddFromResource( 
            /* [in] */ HINSTANCE hInstance,
            /* [in] */ UINT nID) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE AddFromXML( 
            /* [in] */ ISXMLElement *pXML) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetMenu( 
            /* [in] */ BSTR bsName,
            /* [in] */ VARIANT_BOOL bCreate,
            /* [retval][out] */ ISMenu **ppMenu) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE GetToolbar( 
            /* [in] */ BSTR bsName,
            /* [in] */ VARIANT_BOOL bCreate,
            /* [retval][out] */ ISToolbar **ppToolbar) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUserInterfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUserInterface * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUserInterface * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUserInterface * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IUserInterface * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IUserInterface * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IUserInterface * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IUserInterface * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            IUserInterface * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UserInterface )( 
            IUserInterface * This,
            /* [retval][out] */ IUserInterface **ppUserInterface);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *NewWindow )( 
            IUserInterface * This,
            /* [in] */ BSTR bsName,
            /* [in] */ IPluginWindowOwner *pOwner,
            /* [retval][out] */ IPluginWindow **ppWindow);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_MainWindowHwnd )( 
            IUserInterface * This,
            /* [retval][out] */ HWND *phWnd);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ActiveView )( 
            IUserInterface * This,
            /* [retval][out] */ IGenericView **ppView);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *RegisterCommand )( 
            IUserInterface * This,
            /* [in] */ BSTR bsName,
            /* [in] */ HICON hIcon,
            /* [retval][out] */ UINT *pnCommand);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *AddFromString )( 
            IUserInterface * This,
            /* [in] */ BSTR sXML);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *AddFromResource )( 
            IUserInterface * This,
            /* [in] */ HINSTANCE hInstance,
            /* [in] */ UINT nID);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *AddFromXML )( 
            IUserInterface * This,
            /* [in] */ ISXMLElement *pXML);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetMenu )( 
            IUserInterface * This,
            /* [in] */ BSTR bsName,
            /* [in] */ VARIANT_BOOL bCreate,
            /* [retval][out] */ ISMenu **ppMenu);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *GetToolbar )( 
            IUserInterface * This,
            /* [in] */ BSTR bsName,
            /* [in] */ VARIANT_BOOL bCreate,
            /* [retval][out] */ ISToolbar **ppToolbar);
        
        END_INTERFACE
    } IUserInterfaceVtbl;

    interface IUserInterface
    {
        CONST_VTBL struct IUserInterfaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUserInterface_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUserInterface_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUserInterface_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUserInterface_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUserInterface_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUserInterface_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUserInterface_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUserInterface_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define IUserInterface_get_UserInterface(This,ppUserInterface)	\
    (This)->lpVtbl -> get_UserInterface(This,ppUserInterface)

#define IUserInterface_NewWindow(This,bsName,pOwner,ppWindow)	\
    (This)->lpVtbl -> NewWindow(This,bsName,pOwner,ppWindow)

#define IUserInterface_get_MainWindowHwnd(This,phWnd)	\
    (This)->lpVtbl -> get_MainWindowHwnd(This,phWnd)

#define IUserInterface_get_ActiveView(This,ppView)	\
    (This)->lpVtbl -> get_ActiveView(This,ppView)

#define IUserInterface_RegisterCommand(This,bsName,hIcon,pnCommand)	\
    (This)->lpVtbl -> RegisterCommand(This,bsName,hIcon,pnCommand)

#define IUserInterface_AddFromString(This,sXML)	\
    (This)->lpVtbl -> AddFromString(This,sXML)

#define IUserInterface_AddFromResource(This,hInstance,nID)	\
    (This)->lpVtbl -> AddFromResource(This,hInstance,nID)

#define IUserInterface_AddFromXML(This,pXML)	\
    (This)->lpVtbl -> AddFromXML(This,pXML)

#define IUserInterface_GetMenu(This,bsName,bCreate,ppMenu)	\
    (This)->lpVtbl -> GetMenu(This,bsName,bCreate,ppMenu)

#define IUserInterface_GetToolbar(This,bsName,bCreate,ppToolbar)	\
    (This)->lpVtbl -> GetToolbar(This,bsName,bCreate,ppToolbar)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IUserInterface_get_Application_Proxy( 
    IUserInterface * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB IUserInterface_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IUserInterface_get_UserInterface_Proxy( 
    IUserInterface * This,
    /* [retval][out] */ IUserInterface **ppUserInterface);


void __RPC_STUB IUserInterface_get_UserInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IUserInterface_NewWindow_Proxy( 
    IUserInterface * This,
    /* [in] */ BSTR bsName,
    /* [in] */ IPluginWindowOwner *pOwner,
    /* [retval][out] */ IPluginWindow **ppWindow);


void __RPC_STUB IUserInterface_NewWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IUserInterface_get_MainWindowHwnd_Proxy( 
    IUserInterface * This,
    /* [retval][out] */ HWND *phWnd);


void __RPC_STUB IUserInterface_get_MainWindowHwnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IUserInterface_get_ActiveView_Proxy( 
    IUserInterface * This,
    /* [retval][out] */ IGenericView **ppView);


void __RPC_STUB IUserInterface_get_ActiveView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IUserInterface_RegisterCommand_Proxy( 
    IUserInterface * This,
    /* [in] */ BSTR bsName,
    /* [in] */ HICON hIcon,
    /* [retval][out] */ UINT *pnCommand);


void __RPC_STUB IUserInterface_RegisterCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IUserInterface_AddFromString_Proxy( 
    IUserInterface * This,
    /* [in] */ BSTR sXML);


void __RPC_STUB IUserInterface_AddFromString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IUserInterface_AddFromResource_Proxy( 
    IUserInterface * This,
    /* [in] */ HINSTANCE hInstance,
    /* [in] */ UINT nID);


void __RPC_STUB IUserInterface_AddFromResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IUserInterface_AddFromXML_Proxy( 
    IUserInterface * This,
    /* [in] */ ISXMLElement *pXML);


void __RPC_STUB IUserInterface_AddFromXML_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IUserInterface_GetMenu_Proxy( 
    IUserInterface * This,
    /* [in] */ BSTR bsName,
    /* [in] */ VARIANT_BOOL bCreate,
    /* [retval][out] */ ISMenu **ppMenu);


void __RPC_STUB IUserInterface_GetMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IUserInterface_GetToolbar_Proxy( 
    IUserInterface * This,
    /* [in] */ BSTR bsName,
    /* [in] */ VARIANT_BOOL bCreate,
    /* [retval][out] */ ISToolbar **ppToolbar);


void __RPC_STUB IUserInterface_GetToolbar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IUserInterface_INTERFACE_DEFINED__ */


#ifndef __ILibrary_INTERFACE_DEFINED__
#define __ILibrary_INTERFACE_DEFINED__

/* interface ILibrary */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ILibrary,0x1735A63C,0x099B,0x414c,0x9B,0x22,0x81,0x7C,0x2F,0xC5,0xEC,0x34);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1735A63C-099B-414c-9B22-817C2FC5EC34")
    ILibrary : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Library( 
            /* [retval][out] */ ILibrary **ppLibrary) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Folders( 
            /* [retval][out] */ ILibraryFolders **ppFolders) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Albums( 
            /* [retval][out] */ IUnknown **ppAlbums) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Files( 
            /* [retval][out] */ ILibraryFiles **ppFiles) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindByName( 
            /* [in] */ BSTR sName,
            /* [retval][out] */ ILibraryFile **ppFile) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindByPath( 
            /* [in] */ BSTR sPath,
            /* [retval][out] */ ILibraryFile **ppFile) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindByURN( 
            /* [in] */ BSTR sURN,
            /* [retval][out] */ ILibraryFile **ppFile) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE FindByIndex( 
            /* [in] */ LONG nIndex,
            /* [retval][out] */ ILibraryFile **ppFile) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILibraryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILibrary * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILibrary * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILibrary * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILibrary * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILibrary * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILibrary * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILibrary * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ILibrary * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Library )( 
            ILibrary * This,
            /* [retval][out] */ ILibrary **ppLibrary);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Folders )( 
            ILibrary * This,
            /* [retval][out] */ ILibraryFolders **ppFolders);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Albums )( 
            ILibrary * This,
            /* [retval][out] */ IUnknown **ppAlbums);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Files )( 
            ILibrary * This,
            /* [retval][out] */ ILibraryFiles **ppFiles);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindByName )( 
            ILibrary * This,
            /* [in] */ BSTR sName,
            /* [retval][out] */ ILibraryFile **ppFile);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindByPath )( 
            ILibrary * This,
            /* [in] */ BSTR sPath,
            /* [retval][out] */ ILibraryFile **ppFile);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindByURN )( 
            ILibrary * This,
            /* [in] */ BSTR sURN,
            /* [retval][out] */ ILibraryFile **ppFile);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *FindByIndex )( 
            ILibrary * This,
            /* [in] */ LONG nIndex,
            /* [retval][out] */ ILibraryFile **ppFile);
        
        END_INTERFACE
    } ILibraryVtbl;

    interface ILibrary
    {
        CONST_VTBL struct ILibraryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILibrary_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILibrary_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILibrary_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILibrary_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILibrary_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILibrary_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILibrary_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILibrary_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ILibrary_get_Library(This,ppLibrary)	\
    (This)->lpVtbl -> get_Library(This,ppLibrary)

#define ILibrary_get_Folders(This,ppFolders)	\
    (This)->lpVtbl -> get_Folders(This,ppFolders)

#define ILibrary_get_Albums(This,ppAlbums)	\
    (This)->lpVtbl -> get_Albums(This,ppAlbums)

#define ILibrary_get_Files(This,ppFiles)	\
    (This)->lpVtbl -> get_Files(This,ppFiles)

#define ILibrary_FindByName(This,sName,ppFile)	\
    (This)->lpVtbl -> FindByName(This,sName,ppFile)

#define ILibrary_FindByPath(This,sPath,ppFile)	\
    (This)->lpVtbl -> FindByPath(This,sPath,ppFile)

#define ILibrary_FindByURN(This,sURN,ppFile)	\
    (This)->lpVtbl -> FindByURN(This,sURN,ppFile)

#define ILibrary_FindByIndex(This,nIndex,ppFile)	\
    (This)->lpVtbl -> FindByIndex(This,nIndex,ppFile)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibrary_get_Application_Proxy( 
    ILibrary * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ILibrary_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibrary_get_Library_Proxy( 
    ILibrary * This,
    /* [retval][out] */ ILibrary **ppLibrary);


void __RPC_STUB ILibrary_get_Library_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibrary_get_Folders_Proxy( 
    ILibrary * This,
    /* [retval][out] */ ILibraryFolders **ppFolders);


void __RPC_STUB ILibrary_get_Folders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibrary_get_Albums_Proxy( 
    ILibrary * This,
    /* [retval][out] */ IUnknown **ppAlbums);


void __RPC_STUB ILibrary_get_Albums_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibrary_get_Files_Proxy( 
    ILibrary * This,
    /* [retval][out] */ ILibraryFiles **ppFiles);


void __RPC_STUB ILibrary_get_Files_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibrary_FindByName_Proxy( 
    ILibrary * This,
    /* [in] */ BSTR sName,
    /* [retval][out] */ ILibraryFile **ppFile);


void __RPC_STUB ILibrary_FindByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibrary_FindByPath_Proxy( 
    ILibrary * This,
    /* [in] */ BSTR sPath,
    /* [retval][out] */ ILibraryFile **ppFile);


void __RPC_STUB ILibrary_FindByPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibrary_FindByURN_Proxy( 
    ILibrary * This,
    /* [in] */ BSTR sURN,
    /* [retval][out] */ ILibraryFile **ppFile);


void __RPC_STUB ILibrary_FindByURN_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibrary_FindByIndex_Proxy( 
    ILibrary * This,
    /* [in] */ LONG nIndex,
    /* [retval][out] */ ILibraryFile **ppFile);


void __RPC_STUB ILibrary_FindByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILibrary_INTERFACE_DEFINED__ */


#ifndef __ISMenu_INTERFACE_DEFINED__
#define __ISMenu_INTERFACE_DEFINED__

/* interface ISMenu */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISMenu,0xD8C3C592,0x5AF4,0x44cf,0x8A,0xA8,0x59,0x03,0x8F,0xE3,0x88,0x12);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D8C3C592-5AF4-44cf-8AA8-59038FE38812")
    ISMenu : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UserInterface( 
            /* [retval][out] */ IUserInterface **ppUserInterface) = 0;
        
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISMenu **ppMenu) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ItemType( 
            /* [retval][out] */ SMenuType *pnType) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_CommandID( 
            /* [retval][out] */ LONG *pnCommandID) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_CommandID( 
            /* [in] */ LONG nCommandID) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Text( 
            /* [retval][out] */ BSTR *psText) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Text( 
            /* [in] */ BSTR sText) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_HotKey( 
            /* [retval][out] */ BSTR *psText) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_HotKey( 
            /* [in] */ BSTR sText) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InsertSeparator( 
            /* [in] */ LONG nPosition) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InsertMenu( 
            /* [in] */ LONG nPosition,
            /* [in] */ BSTR sText,
            /* [retval][out] */ ISMenu **ppMenu) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE InsertCommand( 
            /* [in] */ LONG nPosition,
            /* [in] */ LONG nCommandID,
            /* [in] */ BSTR sText,
            /* [retval][out] */ ISMenu **ppMenu) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISMenuVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISMenu * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISMenu * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISMenu * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISMenu * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISMenu * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISMenu * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISMenu * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ISMenu * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UserInterface )( 
            ISMenu * This,
            /* [retval][out] */ IUserInterface **ppUserInterface);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISMenu * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISMenu * This,
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ISMenu **ppMenu);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISMenu * This,
            /* [retval][out] */ LONG *pnCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ItemType )( 
            ISMenu * This,
            /* [retval][out] */ SMenuType *pnType);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_CommandID )( 
            ISMenu * This,
            /* [retval][out] */ LONG *pnCommandID);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_CommandID )( 
            ISMenu * This,
            /* [in] */ LONG nCommandID);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Text )( 
            ISMenu * This,
            /* [retval][out] */ BSTR *psText);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Text )( 
            ISMenu * This,
            /* [in] */ BSTR sText);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_HotKey )( 
            ISMenu * This,
            /* [retval][out] */ BSTR *psText);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_HotKey )( 
            ISMenu * This,
            /* [in] */ BSTR sText);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISMenu * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InsertSeparator )( 
            ISMenu * This,
            /* [in] */ LONG nPosition);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InsertMenu )( 
            ISMenu * This,
            /* [in] */ LONG nPosition,
            /* [in] */ BSTR sText,
            /* [retval][out] */ ISMenu **ppMenu);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *InsertCommand )( 
            ISMenu * This,
            /* [in] */ LONG nPosition,
            /* [in] */ LONG nCommandID,
            /* [in] */ BSTR sText,
            /* [retval][out] */ ISMenu **ppMenu);
        
        END_INTERFACE
    } ISMenuVtbl;

    interface ISMenu
    {
        CONST_VTBL struct ISMenuVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISMenu_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISMenu_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISMenu_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISMenu_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISMenu_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISMenu_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISMenu_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISMenu_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ISMenu_get_UserInterface(This,ppUserInterface)	\
    (This)->lpVtbl -> get_UserInterface(This,ppUserInterface)

#define ISMenu_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ISMenu_get_Item(This,vIndex,ppMenu)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppMenu)

#define ISMenu_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#define ISMenu_get_ItemType(This,pnType)	\
    (This)->lpVtbl -> get_ItemType(This,pnType)

#define ISMenu_get_CommandID(This,pnCommandID)	\
    (This)->lpVtbl -> get_CommandID(This,pnCommandID)

#define ISMenu_put_CommandID(This,nCommandID)	\
    (This)->lpVtbl -> put_CommandID(This,nCommandID)

#define ISMenu_get_Text(This,psText)	\
    (This)->lpVtbl -> get_Text(This,psText)

#define ISMenu_put_Text(This,sText)	\
    (This)->lpVtbl -> put_Text(This,sText)

#define ISMenu_get_HotKey(This,psText)	\
    (This)->lpVtbl -> get_HotKey(This,psText)

#define ISMenu_put_HotKey(This,sText)	\
    (This)->lpVtbl -> put_HotKey(This,sText)

#define ISMenu_Remove(This)	\
    (This)->lpVtbl -> Remove(This)

#define ISMenu_InsertSeparator(This,nPosition)	\
    (This)->lpVtbl -> InsertSeparator(This,nPosition)

#define ISMenu_InsertMenu(This,nPosition,sText,ppMenu)	\
    (This)->lpVtbl -> InsertMenu(This,nPosition,sText,ppMenu)

#define ISMenu_InsertCommand(This,nPosition,nCommandID,sText,ppMenu)	\
    (This)->lpVtbl -> InsertCommand(This,nPosition,nCommandID,sText,ppMenu)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_Application_Proxy( 
    ISMenu * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ISMenu_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_UserInterface_Proxy( 
    ISMenu * This,
    /* [retval][out] */ IUserInterface **ppUserInterface);


void __RPC_STUB ISMenu_get_UserInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get__NewEnum_Proxy( 
    ISMenu * This,
    /* [retval][out] */ IUnknown **ppEnum);


void __RPC_STUB ISMenu_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_Item_Proxy( 
    ISMenu * This,
    /* [in] */ VARIANT vIndex,
    /* [retval][out] */ ISMenu **ppMenu);


void __RPC_STUB ISMenu_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_Count_Proxy( 
    ISMenu * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB ISMenu_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_ItemType_Proxy( 
    ISMenu * This,
    /* [retval][out] */ SMenuType *pnType);


void __RPC_STUB ISMenu_get_ItemType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_CommandID_Proxy( 
    ISMenu * This,
    /* [retval][out] */ LONG *pnCommandID);


void __RPC_STUB ISMenu_get_CommandID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISMenu_put_CommandID_Proxy( 
    ISMenu * This,
    /* [in] */ LONG nCommandID);


void __RPC_STUB ISMenu_put_CommandID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_Text_Proxy( 
    ISMenu * This,
    /* [retval][out] */ BSTR *psText);


void __RPC_STUB ISMenu_get_Text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISMenu_put_Text_Proxy( 
    ISMenu * This,
    /* [in] */ BSTR sText);


void __RPC_STUB ISMenu_put_Text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISMenu_get_HotKey_Proxy( 
    ISMenu * This,
    /* [retval][out] */ BSTR *psText);


void __RPC_STUB ISMenu_get_HotKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISMenu_put_HotKey_Proxy( 
    ISMenu * This,
    /* [in] */ BSTR sText);


void __RPC_STUB ISMenu_put_HotKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISMenu_Remove_Proxy( 
    ISMenu * This);


void __RPC_STUB ISMenu_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISMenu_InsertSeparator_Proxy( 
    ISMenu * This,
    /* [in] */ LONG nPosition);


void __RPC_STUB ISMenu_InsertSeparator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISMenu_InsertMenu_Proxy( 
    ISMenu * This,
    /* [in] */ LONG nPosition,
    /* [in] */ BSTR sText,
    /* [retval][out] */ ISMenu **ppMenu);


void __RPC_STUB ISMenu_InsertMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISMenu_InsertCommand_Proxy( 
    ISMenu * This,
    /* [in] */ LONG nPosition,
    /* [in] */ LONG nCommandID,
    /* [in] */ BSTR sText,
    /* [retval][out] */ ISMenu **ppMenu);


void __RPC_STUB ISMenu_InsertCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISMenu_INTERFACE_DEFINED__ */


#ifndef __ISToolbarItem_INTERFACE_DEFINED__
#define __ISToolbarItem_INTERFACE_DEFINED__

/* interface ISToolbarItem */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ISToolbarItem,0x3A82A3A3,0x5560,0x4ece,0xB3,0x8A,0xD5,0x6E,0x1E,0x74,0x64,0x2A);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3A82A3A3-5560-4ece-B38A-D56E1E74642A")
    ISToolbarItem : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_UserInterface( 
            /* [retval][out] */ IUserInterface **ppUserInterface) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Toolbar( 
            /* [retval][out] */ ISToolbar **ppToolbar) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ItemType( 
            /* [retval][out] */ SToolbarType *pnType) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_CommandID( 
            /* [retval][out] */ LONG *pnCommandID) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_CommandID( 
            /* [in] */ LONG nCommandID) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Text( 
            /* [retval][out] */ BSTR *psText) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Text( 
            /* [in] */ BSTR sText) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISToolbarItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISToolbarItem * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISToolbarItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISToolbarItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISToolbarItem * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISToolbarItem * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISToolbarItem * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISToolbarItem * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ISToolbarItem * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_UserInterface )( 
            ISToolbarItem * This,
            /* [retval][out] */ IUserInterface **ppUserInterface);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Toolbar )( 
            ISToolbarItem * This,
            /* [retval][out] */ ISToolbar **ppToolbar);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ItemType )( 
            ISToolbarItem * This,
            /* [retval][out] */ SToolbarType *pnType);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_CommandID )( 
            ISToolbarItem * This,
            /* [retval][out] */ LONG *pnCommandID);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_CommandID )( 
            ISToolbarItem * This,
            /* [in] */ LONG nCommandID);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Text )( 
            ISToolbarItem * This,
            /* [retval][out] */ BSTR *psText);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Text )( 
            ISToolbarItem * This,
            /* [in] */ BSTR sText);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISToolbarItem * This);
        
        END_INTERFACE
    } ISToolbarItemVtbl;

    interface ISToolbarItem
    {
        CONST_VTBL struct ISToolbarItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISToolbarItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISToolbarItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISToolbarItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISToolbarItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISToolbarItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISToolbarItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISToolbarItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISToolbarItem_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ISToolbarItem_get_UserInterface(This,ppUserInterface)	\
    (This)->lpVtbl -> get_UserInterface(This,ppUserInterface)

#define ISToolbarItem_get_Toolbar(This,ppToolbar)	\
    (This)->lpVtbl -> get_Toolbar(This,ppToolbar)

#define ISToolbarItem_get_ItemType(This,pnType)	\
    (This)->lpVtbl -> get_ItemType(This,pnType)

#define ISToolbarItem_get_CommandID(This,pnCommandID)	\
    (This)->lpVtbl -> get_CommandID(This,pnCommandID)

#define ISToolbarItem_put_CommandID(This,nCommandID)	\
    (This)->lpVtbl -> put_CommandID(This,nCommandID)

#define ISToolbarItem_get_Text(This,psText)	\
    (This)->lpVtbl -> get_Text(This,psText)

#define ISToolbarItem_put_Text(This,sText)	\
    (This)->lpVtbl -> put_Text(This,sText)

#define ISToolbarItem_Remove(This)	\
    (This)->lpVtbl -> Remove(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_get_Application_Proxy( 
    ISToolbarItem * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ISToolbarItem_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_get_UserInterface_Proxy( 
    ISToolbarItem * This,
    /* [retval][out] */ IUserInterface **ppUserInterface);


void __RPC_STUB ISToolbarItem_get_UserInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_get_Toolbar_Proxy( 
    ISToolbarItem * This,
    /* [retval][out] */ ISToolbar **ppToolbar);


void __RPC_STUB ISToolbarItem_get_Toolbar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_get_ItemType_Proxy( 
    ISToolbarItem * This,
    /* [retval][out] */ SToolbarType *pnType);


void __RPC_STUB ISToolbarItem_get_ItemType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_get_CommandID_Proxy( 
    ISToolbarItem * This,
    /* [retval][out] */ LONG *pnCommandID);


void __RPC_STUB ISToolbarItem_get_CommandID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_put_CommandID_Proxy( 
    ISToolbarItem * This,
    /* [in] */ LONG nCommandID);


void __RPC_STUB ISToolbarItem_put_CommandID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_get_Text_Proxy( 
    ISToolbarItem * This,
    /* [retval][out] */ BSTR *psText);


void __RPC_STUB ISToolbarItem_get_Text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_put_Text_Proxy( 
    ISToolbarItem * This,
    /* [in] */ BSTR sText);


void __RPC_STUB ISToolbarItem_put_Text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ISToolbarItem_Remove_Proxy( 
    ISToolbarItem * This);


void __RPC_STUB ISToolbarItem_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISToolbarItem_INTERFACE_DEFINED__ */


#ifndef __IGenericView_INTERFACE_DEFINED__
#define __IGenericView_INTERFACE_DEFINED__

/* interface IGenericView */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_IGenericView,0xEBAD02A1,0xE1B0,0x4961,0x94,0x15,0x83,0x26,0x7B,0x2A,0x50,0x10);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EBAD02A1-E1B0-4961-9415-83267B2A5010")
    IGenericView : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *psName) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Unknown( 
            /* [retval][out] */ IUnknown **ppUnknown) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Param( 
            /* [retval][out] */ LONG *pnParam) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ VARIANT *pvItem) = 0;
        
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGenericViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGenericView * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGenericView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGenericView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGenericView * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGenericView * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGenericView * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGenericView * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IGenericView * This,
            /* [retval][out] */ BSTR *psName);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Unknown )( 
            IGenericView * This,
            /* [retval][out] */ IUnknown **ppUnknown);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Param )( 
            IGenericView * This,
            /* [retval][out] */ LONG *pnParam);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IGenericView * This,
            /* [retval][out] */ LONG *pnCount);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IGenericView * This,
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ VARIANT *pvItem);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IGenericView * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        END_INTERFACE
    } IGenericViewVtbl;

    interface IGenericView
    {
        CONST_VTBL struct IGenericViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGenericView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGenericView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGenericView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGenericView_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGenericView_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGenericView_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGenericView_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGenericView_get_Name(This,psName)	\
    (This)->lpVtbl -> get_Name(This,psName)

#define IGenericView_get_Unknown(This,ppUnknown)	\
    (This)->lpVtbl -> get_Unknown(This,ppUnknown)

#define IGenericView_get_Param(This,pnParam)	\
    (This)->lpVtbl -> get_Param(This,pnParam)

#define IGenericView_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#define IGenericView_get_Item(This,vIndex,pvItem)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pvItem)

#define IGenericView_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IGenericView_get_Name_Proxy( 
    IGenericView * This,
    /* [retval][out] */ BSTR *psName);


void __RPC_STUB IGenericView_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IGenericView_get_Unknown_Proxy( 
    IGenericView * This,
    /* [retval][out] */ IUnknown **ppUnknown);


void __RPC_STUB IGenericView_get_Unknown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IGenericView_get_Param_Proxy( 
    IGenericView * This,
    /* [retval][out] */ LONG *pnParam);


void __RPC_STUB IGenericView_get_Param_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IGenericView_get_Count_Proxy( 
    IGenericView * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB IGenericView_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IGenericView_get_Item_Proxy( 
    IGenericView * This,
    /* [in] */ VARIANT vIndex,
    /* [retval][out] */ VARIANT *pvItem);


void __RPC_STUB IGenericView_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE IGenericView_get__NewEnum_Proxy( 
    IGenericView * This,
    /* [retval][out] */ IUnknown **ppEnum);


void __RPC_STUB IGenericView_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGenericView_INTERFACE_DEFINED__ */


#ifndef __ILibraryFile_INTERFACE_DEFINED__
#define __ILibraryFile_INTERFACE_DEFINED__

/* interface ILibraryFile */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ILibraryFile,0xB663F7DE,0xE1C6,0x4fe6,0x92,0xBD,0xD0,0x54,0x9B,0x76,0x01,0xE3);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B663F7DE-E1C6-4fe6-92BD-D0549B7601E3")
    ILibraryFile : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Library( 
            /* [retval][out] */ ILibrary **ppLibrary) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Folder( 
            /* [retval][out] */ ILibraryFolder **ppFolder) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Path( 
            /* [retval][out] */ BSTR *psPath) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *psPath) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Shared( 
            /* [retval][out] */ STRISTATE *pnValue) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Shared( 
            /* [in] */ STRISTATE nValue) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_EffectiveShared( 
            /* [retval][out] */ VARIANT_BOOL *pbValue) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Size( 
            /* [retval][out] */ LONG *pnSize) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Index( 
            /* [retval][out] */ LONG *pnIndex) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_URN( 
            /* [in] */ BSTR sURN,
            /* [retval][out] */ BSTR *psURN) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_MetadataAuto( 
            /* [retval][out] */ VARIANT_BOOL *pbValue) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Metadata( 
            /* [retval][out] */ ISXMLElement **ppXML) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Metadata( 
            /* [in] */ ISXMLElement *pXML) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Execute( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE SmartExecute( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Rename( 
            /* [in] */ BSTR sNewName) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ BSTR sNewPath) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Move( 
            /* [in] */ BSTR sNewPath) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILibraryFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILibraryFile * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILibraryFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILibraryFile * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILibraryFile * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILibraryFile * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILibraryFile * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILibraryFile * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ILibraryFile * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Library )( 
            ILibraryFile * This,
            /* [retval][out] */ ILibrary **ppLibrary);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Folder )( 
            ILibraryFile * This,
            /* [retval][out] */ ILibraryFolder **ppFolder);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            ILibraryFile * This,
            /* [retval][out] */ BSTR *psPath);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ILibraryFile * This,
            /* [retval][out] */ BSTR *psPath);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Shared )( 
            ILibraryFile * This,
            /* [retval][out] */ STRISTATE *pnValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Shared )( 
            ILibraryFile * This,
            /* [in] */ STRISTATE nValue);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_EffectiveShared )( 
            ILibraryFile * This,
            /* [retval][out] */ VARIANT_BOOL *pbValue);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            ILibraryFile * This,
            /* [retval][out] */ LONG *pnSize);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Index )( 
            ILibraryFile * This,
            /* [retval][out] */ LONG *pnIndex);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_URN )( 
            ILibraryFile * This,
            /* [in] */ BSTR sURN,
            /* [retval][out] */ BSTR *psURN);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_MetadataAuto )( 
            ILibraryFile * This,
            /* [retval][out] */ VARIANT_BOOL *pbValue);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Metadata )( 
            ILibraryFile * This,
            /* [retval][out] */ ISXMLElement **ppXML);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Metadata )( 
            ILibraryFile * This,
            /* [in] */ ISXMLElement *pXML);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Execute )( 
            ILibraryFile * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *SmartExecute )( 
            ILibraryFile * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Delete )( 
            ILibraryFile * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Rename )( 
            ILibraryFile * This,
            /* [in] */ BSTR sNewName);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            ILibraryFile * This,
            /* [in] */ BSTR sNewPath);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Move )( 
            ILibraryFile * This,
            /* [in] */ BSTR sNewPath);
        
        END_INTERFACE
    } ILibraryFileVtbl;

    interface ILibraryFile
    {
        CONST_VTBL struct ILibraryFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILibraryFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILibraryFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILibraryFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILibraryFile_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILibraryFile_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILibraryFile_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILibraryFile_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILibraryFile_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ILibraryFile_get_Library(This,ppLibrary)	\
    (This)->lpVtbl -> get_Library(This,ppLibrary)

#define ILibraryFile_get_Folder(This,ppFolder)	\
    (This)->lpVtbl -> get_Folder(This,ppFolder)

#define ILibraryFile_get_Path(This,psPath)	\
    (This)->lpVtbl -> get_Path(This,psPath)

#define ILibraryFile_get_Name(This,psPath)	\
    (This)->lpVtbl -> get_Name(This,psPath)

#define ILibraryFile_get_Shared(This,pnValue)	\
    (This)->lpVtbl -> get_Shared(This,pnValue)

#define ILibraryFile_put_Shared(This,nValue)	\
    (This)->lpVtbl -> put_Shared(This,nValue)

#define ILibraryFile_get_EffectiveShared(This,pbValue)	\
    (This)->lpVtbl -> get_EffectiveShared(This,pbValue)

#define ILibraryFile_get_Size(This,pnSize)	\
    (This)->lpVtbl -> get_Size(This,pnSize)

#define ILibraryFile_get_Index(This,pnIndex)	\
    (This)->lpVtbl -> get_Index(This,pnIndex)

#define ILibraryFile_get_URN(This,sURN,psURN)	\
    (This)->lpVtbl -> get_URN(This,sURN,psURN)

#define ILibraryFile_get_MetadataAuto(This,pbValue)	\
    (This)->lpVtbl -> get_MetadataAuto(This,pbValue)

#define ILibraryFile_get_Metadata(This,ppXML)	\
    (This)->lpVtbl -> get_Metadata(This,ppXML)

#define ILibraryFile_put_Metadata(This,pXML)	\
    (This)->lpVtbl -> put_Metadata(This,pXML)

#define ILibraryFile_Execute(This)	\
    (This)->lpVtbl -> Execute(This)

#define ILibraryFile_SmartExecute(This)	\
    (This)->lpVtbl -> SmartExecute(This)

#define ILibraryFile_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define ILibraryFile_Rename(This,sNewName)	\
    (This)->lpVtbl -> Rename(This,sNewName)

#define ILibraryFile_Copy(This,sNewPath)	\
    (This)->lpVtbl -> Copy(This,sNewPath)

#define ILibraryFile_Move(This,sNewPath)	\
    (This)->lpVtbl -> Move(This,sNewPath)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Application_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ILibraryFile_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Library_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ ILibrary **ppLibrary);


void __RPC_STUB ILibraryFile_get_Library_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Folder_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ ILibraryFolder **ppFolder);


void __RPC_STUB ILibraryFile_get_Folder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Path_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ BSTR *psPath);


void __RPC_STUB ILibraryFile_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Name_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ BSTR *psPath);


void __RPC_STUB ILibraryFile_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Shared_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ STRISTATE *pnValue);


void __RPC_STUB ILibraryFile_get_Shared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_put_Shared_Proxy( 
    ILibraryFile * This,
    /* [in] */ STRISTATE nValue);


void __RPC_STUB ILibraryFile_put_Shared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_EffectiveShared_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ VARIANT_BOOL *pbValue);


void __RPC_STUB ILibraryFile_get_EffectiveShared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Size_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ LONG *pnSize);


void __RPC_STUB ILibraryFile_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Index_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ LONG *pnIndex);


void __RPC_STUB ILibraryFile_get_Index_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_URN_Proxy( 
    ILibraryFile * This,
    /* [in] */ BSTR sURN,
    /* [retval][out] */ BSTR *psURN);


void __RPC_STUB ILibraryFile_get_URN_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_MetadataAuto_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ VARIANT_BOOL *pbValue);


void __RPC_STUB ILibraryFile_get_MetadataAuto_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_get_Metadata_Proxy( 
    ILibraryFile * This,
    /* [retval][out] */ ISXMLElement **ppXML);


void __RPC_STUB ILibraryFile_get_Metadata_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_put_Metadata_Proxy( 
    ILibraryFile * This,
    /* [in] */ ISXMLElement *pXML);


void __RPC_STUB ILibraryFile_put_Metadata_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_Execute_Proxy( 
    ILibraryFile * This);


void __RPC_STUB ILibraryFile_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_SmartExecute_Proxy( 
    ILibraryFile * This);


void __RPC_STUB ILibraryFile_SmartExecute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_Delete_Proxy( 
    ILibraryFile * This);


void __RPC_STUB ILibraryFile_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_Rename_Proxy( 
    ILibraryFile * This,
    /* [in] */ BSTR sNewName);


void __RPC_STUB ILibraryFile_Rename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_Copy_Proxy( 
    ILibraryFile * This,
    /* [in] */ BSTR sNewPath);


void __RPC_STUB ILibraryFile_Copy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ILibraryFile_Move_Proxy( 
    ILibraryFile * This,
    /* [in] */ BSTR sNewPath);


void __RPC_STUB ILibraryFile_Move_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILibraryFile_INTERFACE_DEFINED__ */


#ifndef __ILibraryFiles_INTERFACE_DEFINED__
#define __ILibraryFiles_INTERFACE_DEFINED__

/* interface ILibraryFiles */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ILibraryFiles,0x49346C06,0xDC33,0x4975,0x97,0x8E,0xE8,0x07,0xF7,0xE4,0x1E,0xF9);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49346C06-DC33-4975-978E-E807F7E41EF9")
    ILibraryFiles : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Library( 
            /* [retval][out] */ ILibrary **ppLibrary) = 0;
        
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ILibraryFile **ppFile) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILibraryFilesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILibraryFiles * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILibraryFiles * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILibraryFiles * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILibraryFiles * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILibraryFiles * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILibraryFiles * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILibraryFiles * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ILibraryFiles * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Library )( 
            ILibraryFiles * This,
            /* [retval][out] */ ILibrary **ppLibrary);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ILibraryFiles * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ILibraryFiles * This,
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ILibraryFile **ppFile);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ILibraryFiles * This,
            /* [retval][out] */ LONG *pnCount);
        
        END_INTERFACE
    } ILibraryFilesVtbl;

    interface ILibraryFiles
    {
        CONST_VTBL struct ILibraryFilesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILibraryFiles_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILibraryFiles_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILibraryFiles_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILibraryFiles_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILibraryFiles_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILibraryFiles_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILibraryFiles_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILibraryFiles_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ILibraryFiles_get_Library(This,ppLibrary)	\
    (This)->lpVtbl -> get_Library(This,ppLibrary)

#define ILibraryFiles_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ILibraryFiles_get_Item(This,vIndex,ppFile)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppFile)

#define ILibraryFiles_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFiles_get_Application_Proxy( 
    ILibraryFiles * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ILibraryFiles_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFiles_get_Library_Proxy( 
    ILibraryFiles * This,
    /* [retval][out] */ ILibrary **ppLibrary);


void __RPC_STUB ILibraryFiles_get_Library_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFiles_get__NewEnum_Proxy( 
    ILibraryFiles * This,
    /* [retval][out] */ IUnknown **ppEnum);


void __RPC_STUB ILibraryFiles_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFiles_get_Item_Proxy( 
    ILibraryFiles * This,
    /* [in] */ VARIANT vIndex,
    /* [retval][out] */ ILibraryFile **ppFile);


void __RPC_STUB ILibraryFiles_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFiles_get_Count_Proxy( 
    ILibraryFiles * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB ILibraryFiles_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILibraryFiles_INTERFACE_DEFINED__ */


#ifndef __ILibraryFolder_INTERFACE_DEFINED__
#define __ILibraryFolder_INTERFACE_DEFINED__

/* interface ILibraryFolder */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ILibraryFolder,0x8A6BC680,0x3451,0x4a78,0x8A,0x01,0xB7,0xDF,0xC1,0xD9,0xA1,0x48);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8A6BC680-3451-4a78-8A01-B7DFC1D9A148")
    ILibraryFolder : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Library( 
            /* [retval][out] */ ILibrary **ppLibrary) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ ILibraryFolder **ppFolder) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Path( 
            /* [retval][out] */ BSTR *psPath) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *psPath) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Shared( 
            /* [retval][out] */ STRISTATE *pnValue) = 0;
        
        virtual /* [propput][id] */ HRESULT STDMETHODCALLTYPE put_Shared( 
            /* [in] */ STRISTATE nValue) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_EffectiveShared( 
            /* [retval][out] */ VARIANT_BOOL *pbValue) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Folders( 
            /* [retval][out] */ ILibraryFolders **ppFolders) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Files( 
            /* [retval][out] */ ILibraryFiles **ppFiles) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILibraryFolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILibraryFolder * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILibraryFolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILibraryFolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILibraryFolder * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILibraryFolder * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILibraryFolder * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILibraryFolder * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ILibraryFolder * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Library )( 
            ILibraryFolder * This,
            /* [retval][out] */ ILibrary **ppLibrary);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            ILibraryFolder * This,
            /* [retval][out] */ ILibraryFolder **ppFolder);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            ILibraryFolder * This,
            /* [retval][out] */ BSTR *psPath);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ILibraryFolder * This,
            /* [retval][out] */ BSTR *psPath);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Shared )( 
            ILibraryFolder * This,
            /* [retval][out] */ STRISTATE *pnValue);
        
        /* [propput][id] */ HRESULT ( STDMETHODCALLTYPE *put_Shared )( 
            ILibraryFolder * This,
            /* [in] */ STRISTATE nValue);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_EffectiveShared )( 
            ILibraryFolder * This,
            /* [retval][out] */ VARIANT_BOOL *pbValue);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Folders )( 
            ILibraryFolder * This,
            /* [retval][out] */ ILibraryFolders **ppFolders);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Files )( 
            ILibraryFolder * This,
            /* [retval][out] */ ILibraryFiles **ppFiles);
        
        END_INTERFACE
    } ILibraryFolderVtbl;

    interface ILibraryFolder
    {
        CONST_VTBL struct ILibraryFolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILibraryFolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILibraryFolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILibraryFolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILibraryFolder_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILibraryFolder_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILibraryFolder_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILibraryFolder_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILibraryFolder_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ILibraryFolder_get_Library(This,ppLibrary)	\
    (This)->lpVtbl -> get_Library(This,ppLibrary)

#define ILibraryFolder_get_Parent(This,ppFolder)	\
    (This)->lpVtbl -> get_Parent(This,ppFolder)

#define ILibraryFolder_get_Path(This,psPath)	\
    (This)->lpVtbl -> get_Path(This,psPath)

#define ILibraryFolder_get_Name(This,psPath)	\
    (This)->lpVtbl -> get_Name(This,psPath)

#define ILibraryFolder_get_Shared(This,pnValue)	\
    (This)->lpVtbl -> get_Shared(This,pnValue)

#define ILibraryFolder_put_Shared(This,nValue)	\
    (This)->lpVtbl -> put_Shared(This,nValue)

#define ILibraryFolder_get_EffectiveShared(This,pbValue)	\
    (This)->lpVtbl -> get_EffectiveShared(This,pbValue)

#define ILibraryFolder_get_Folders(This,ppFolders)	\
    (This)->lpVtbl -> get_Folders(This,ppFolders)

#define ILibraryFolder_get_Files(This,ppFiles)	\
    (This)->lpVtbl -> get_Files(This,ppFiles)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Application_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ILibraryFolder_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Library_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ ILibrary **ppLibrary);


void __RPC_STUB ILibraryFolder_get_Library_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Parent_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ ILibraryFolder **ppFolder);


void __RPC_STUB ILibraryFolder_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Path_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ BSTR *psPath);


void __RPC_STUB ILibraryFolder_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Name_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ BSTR *psPath);


void __RPC_STUB ILibraryFolder_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Shared_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ STRISTATE *pnValue);


void __RPC_STUB ILibraryFolder_get_Shared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_put_Shared_Proxy( 
    ILibraryFolder * This,
    /* [in] */ STRISTATE nValue);


void __RPC_STUB ILibraryFolder_put_Shared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_EffectiveShared_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ VARIANT_BOOL *pbValue);


void __RPC_STUB ILibraryFolder_get_EffectiveShared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Folders_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ ILibraryFolders **ppFolders);


void __RPC_STUB ILibraryFolder_get_Folders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolder_get_Files_Proxy( 
    ILibraryFolder * This,
    /* [retval][out] */ ILibraryFiles **ppFiles);


void __RPC_STUB ILibraryFolder_get_Files_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILibraryFolder_INTERFACE_DEFINED__ */


#ifndef __ILibraryFolders_INTERFACE_DEFINED__
#define __ILibraryFolders_INTERFACE_DEFINED__

/* interface ILibraryFolders */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ILibraryFolders,0x43DF8D22,0x6F85,0x4d0a,0xB0,0x72,0x1C,0x8B,0xF8,0xA5,0x70,0x73);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43DF8D22-6F85-4d0a-B072-1C8BF8A57073")
    ILibraryFolders : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Library( 
            /* [retval][out] */ ILibrary **ppLibrary) = 0;
        
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppEnum) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ILibraryFolder **ppFolder) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILibraryFoldersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILibraryFolders * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILibraryFolders * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILibraryFolders * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILibraryFolders * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILibraryFolders * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILibraryFolders * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILibraryFolders * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ILibraryFolders * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Library )( 
            ILibraryFolders * This,
            /* [retval][out] */ ILibrary **ppLibrary);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ILibraryFolders * This,
            /* [retval][out] */ IUnknown **ppEnum);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ILibraryFolders * This,
            /* [in] */ VARIANT vIndex,
            /* [retval][out] */ ILibraryFolder **ppFolder);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ILibraryFolders * This,
            /* [retval][out] */ LONG *pnCount);
        
        END_INTERFACE
    } ILibraryFoldersVtbl;

    interface ILibraryFolders
    {
        CONST_VTBL struct ILibraryFoldersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILibraryFolders_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILibraryFolders_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILibraryFolders_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILibraryFolders_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILibraryFolders_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILibraryFolders_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILibraryFolders_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILibraryFolders_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ILibraryFolders_get_Library(This,ppLibrary)	\
    (This)->lpVtbl -> get_Library(This,ppLibrary)

#define ILibraryFolders_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ILibraryFolders_get_Item(This,vIndex,ppFolder)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppFolder)

#define ILibraryFolders_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolders_get_Application_Proxy( 
    ILibraryFolders * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ILibraryFolders_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolders_get_Library_Proxy( 
    ILibraryFolders * This,
    /* [retval][out] */ ILibrary **ppLibrary);


void __RPC_STUB ILibraryFolders_get_Library_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolders_get__NewEnum_Proxy( 
    ILibraryFolders * This,
    /* [retval][out] */ IUnknown **ppEnum);


void __RPC_STUB ILibraryFolders_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolders_get_Item_Proxy( 
    ILibraryFolders * This,
    /* [in] */ VARIANT vIndex,
    /* [retval][out] */ ILibraryFolder **ppFolder);


void __RPC_STUB ILibraryFolders_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ILibraryFolders_get_Count_Proxy( 
    ILibraryFolders * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB ILibraryFolders_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILibraryFolders_INTERFACE_DEFINED__ */


#ifndef __ICollectionHtmlView_INTERFACE_DEFINED__
#define __ICollectionHtmlView_INTERFACE_DEFINED__

/* interface ICollectionHtmlView */
/* [object][oleautomation][dual][uuid] */ 


DEFINE_GUID(IID_ICollectionHtmlView,0xCF66956E,0x901F,0x44D0,0xB5,0xC6,0xFC,0x6A,0x43,0x6A,0x03,0x0D);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF66956E-901F-44D0-B5C6-FC6A436A030D")
    ICollectionHtmlView : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Application( 
            /* [retval][out] */ IApplication **ppApplication) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Detect( 
            /* [in] */ BSTR sURN,
            /* [retval][out] */ BSTR *psState) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Hover( 
            /* [in] */ BSTR sURN) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ BSTR sURN,
            /* [retval][out] */ VARIANT_BOOL *pbResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Enqueue( 
            /* [in] */ BSTR sURN,
            /* [retval][out] */ VARIANT_BOOL *pbResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Download( 
            /* [in] */ BSTR sMagnet,
            /* [retval][out] */ VARIANT_BOOL *pbResult) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE DownloadAll( void) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_MissingCount( 
            /* [retval][out] */ LONG *pnCount) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICollectionHtmlViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICollectionHtmlView * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICollectionHtmlView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICollectionHtmlView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICollectionHtmlView * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICollectionHtmlView * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICollectionHtmlView * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICollectionHtmlView * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Application )( 
            ICollectionHtmlView * This,
            /* [retval][out] */ IApplication **ppApplication);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Detect )( 
            ICollectionHtmlView * This,
            /* [in] */ BSTR sURN,
            /* [retval][out] */ BSTR *psState);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Hover )( 
            ICollectionHtmlView * This,
            /* [in] */ BSTR sURN);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Open )( 
            ICollectionHtmlView * This,
            /* [in] */ BSTR sURN,
            /* [retval][out] */ VARIANT_BOOL *pbResult);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Enqueue )( 
            ICollectionHtmlView * This,
            /* [in] */ BSTR sURN,
            /* [retval][out] */ VARIANT_BOOL *pbResult);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Download )( 
            ICollectionHtmlView * This,
            /* [in] */ BSTR sMagnet,
            /* [retval][out] */ VARIANT_BOOL *pbResult);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *DownloadAll )( 
            ICollectionHtmlView * This);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_MissingCount )( 
            ICollectionHtmlView * This,
            /* [retval][out] */ LONG *pnCount);
        
        END_INTERFACE
    } ICollectionHtmlViewVtbl;

    interface ICollectionHtmlView
    {
        CONST_VTBL struct ICollectionHtmlViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICollectionHtmlView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICollectionHtmlView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICollectionHtmlView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICollectionHtmlView_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICollectionHtmlView_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICollectionHtmlView_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICollectionHtmlView_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICollectionHtmlView_get_Application(This,ppApplication)	\
    (This)->lpVtbl -> get_Application(This,ppApplication)

#define ICollectionHtmlView_Detect(This,sURN,psState)	\
    (This)->lpVtbl -> Detect(This,sURN,psState)

#define ICollectionHtmlView_Hover(This,sURN)	\
    (This)->lpVtbl -> Hover(This,sURN)

#define ICollectionHtmlView_Open(This,sURN,pbResult)	\
    (This)->lpVtbl -> Open(This,sURN,pbResult)

#define ICollectionHtmlView_Enqueue(This,sURN,pbResult)	\
    (This)->lpVtbl -> Enqueue(This,sURN,pbResult)

#define ICollectionHtmlView_Download(This,sMagnet,pbResult)	\
    (This)->lpVtbl -> Download(This,sMagnet,pbResult)

#define ICollectionHtmlView_DownloadAll(This)	\
    (This)->lpVtbl -> DownloadAll(This)

#define ICollectionHtmlView_get_MissingCount(This,pnCount)	\
    (This)->lpVtbl -> get_MissingCount(This,pnCount)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_get_Application_Proxy( 
    ICollectionHtmlView * This,
    /* [retval][out] */ IApplication **ppApplication);


void __RPC_STUB ICollectionHtmlView_get_Application_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_Detect_Proxy( 
    ICollectionHtmlView * This,
    /* [in] */ BSTR sURN,
    /* [retval][out] */ BSTR *psState);


void __RPC_STUB ICollectionHtmlView_Detect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_Hover_Proxy( 
    ICollectionHtmlView * This,
    /* [in] */ BSTR sURN);


void __RPC_STUB ICollectionHtmlView_Hover_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_Open_Proxy( 
    ICollectionHtmlView * This,
    /* [in] */ BSTR sURN,
    /* [retval][out] */ VARIANT_BOOL *pbResult);


void __RPC_STUB ICollectionHtmlView_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_Enqueue_Proxy( 
    ICollectionHtmlView * This,
    /* [in] */ BSTR sURN,
    /* [retval][out] */ VARIANT_BOOL *pbResult);


void __RPC_STUB ICollectionHtmlView_Enqueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_Download_Proxy( 
    ICollectionHtmlView * This,
    /* [in] */ BSTR sMagnet,
    /* [retval][out] */ VARIANT_BOOL *pbResult);


void __RPC_STUB ICollectionHtmlView_Download_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_DownloadAll_Proxy( 
    ICollectionHtmlView * This);


void __RPC_STUB ICollectionHtmlView_DownloadAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE ICollectionHtmlView_get_MissingCount_Proxy( 
    ICollectionHtmlView * This,
    /* [retval][out] */ LONG *pnCount);


void __RPC_STUB ICollectionHtmlView_get_MissingCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICollectionHtmlView_INTERFACE_DEFINED__ */

#endif /* __Shareaza_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


