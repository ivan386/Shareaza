/*
 *                    Graphics File Library
 *
 *  For Windows & Un*x
 *
 *  GFL library Copyright (c) 1991-2003 Pierre-e Gougelet
 *  All rights reserved
 *
 *
 *  Commercial use is not authorized without agreement
 * 
 *  URL:     http://www.xnview.com
 *  E-Mail : webmaster@xnview.com
 */

#ifndef __GRAPHIC_FILE_LIBRARY_H__
#define __GRAPHIC_FILE_LIBRARY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && !defined(WIN32)
	#define WIN32
#endif

#if defined( WIN32 ) || defined ( __BORLANDC__ )
	#ifdef __BORLANDC__
		#pragma option -a8  /* switch to 8-bytes alignment */
	#elif defined (__MINGW32__)
		#pragma pack (push, before_push, 8)
	#else
		#pragma pack (push, before_push)
		#pragma pack (8)
	#endif
#else
#endif

#if defined( WIN32 ) || defined ( __BORLANDC__ )
	#define GFLEXTERN /*__declspec(dllexport)*/
	#define GFLAPI __stdcall
#else
	#define GFLEXTERN
	#define GFLAPI
#endif

#define GFL_VERSION  "3.40"
#define GFL_VERSION_ 0x0340

#define GFL_FALSE    0
#define GFL_TRUE     1

#if defined(WIN32) && !defined(__MINGW32__)
    #include <basetsd.h>

    typedef INT8     GFL_INT8;
    typedef UINT8    GFL_UINT8;
    typedef INT16    GFL_INT16;
    typedef UINT16   GFL_UINT16;
    typedef INT32    GFL_INT32;
    typedef UINT32   GFL_UINT32;
#else
    #include <stdint.h>

    typedef int8_t     GFL_INT8;
    typedef uint8_t    GFL_UINT8;
    typedef int16_t    GFL_INT16;
    typedef uint16_t   GFL_UINT16;
    typedef int32_t    GFL_INT32;
    typedef uint32_t   GFL_UINT32;
#endif 
/*
typedef signed char    GFL_INT8; 
typedef unsigned char  GFL_UINT8; 
typedef signed short   GFL_INT16; 
typedef unsigned short GFL_UINT16; 
#if __WORDSIZE == 64
typedef signed int     GFL_INT32; 
typedef unsigned int   GFL_UINT32; 
#else
typedef signed long    GFL_INT32; 
typedef unsigned long  GFL_UINT32; 
#endif
*/

typedef unsigned char  GFL_BOOL; 

/*
 *  ERROR
 */
#define GFL_NO_ERROR 0

#define GFL_ERROR_FILE_OPEN         1
#define GFL_ERROR_FILE_READ         2
#define GFL_ERROR_FILE_CREATE       3
#define GFL_ERROR_FILE_WRITE        4
#define GFL_ERROR_NO_MEMORY         5
#define GFL_ERROR_UNKNOWN_FORMAT    6
#define GFL_ERROR_BAD_BITMAP        7
#define GFL_ERROR_BAD_FORMAT_INDEX  10
#define GFL_ERROR_BAD_PARAMETERS    50

#define GFL_UNKNOWN_ERROR           255

typedef GFL_INT16 GFL_ERROR; 

/*
 *  ORIGIN type
 */
#define GFL_LEFT            0x00
#define GFL_RIGHT           0x01
#define GFL_TOP             0x00
#define GFL_BOTTOM          0x10
#define GFL_TOP_LEFT        (GFL_TOP | GFL_LEFT)
#define GFL_BOTTOM_LEFT     (GFL_BOTTOM | GFL_LEFT)
#define GFL_TOP_RIGHT       (GFL_TOP | GFL_RIGHT)
#define GFL_BOTTOM_RIGHT    (GFL_BOTTOM | GFL_RIGHT)

typedef GFL_UINT16 GFL_ORIGIN; 

/*
 *  COMPRESSION type
 */
#define GFL_NO_COMPRESSION        0
#define GFL_RLE                   1
#define GFL_LZW                   2
#define GFL_JPEG                  3
#define GFL_ZIP                   4
#define GFL_SGI_RLE               5
#define GFL_CCITT_RLE             6
#define GFL_CCITT_FAX3            7
#define GFL_CCITT_FAX3_2D         8
#define GFL_CCITT_FAX4            9
#define GFL_WAVELET               10
#define GFL_LZW_PREDICTOR         11
#define GFL_UNKNOWN_COMPRESSION   255

typedef GFL_UINT16 GFL_COMPRESSION; 

/*
 *  BITMAP type
 */
#define GFL_BINARY   0x0001
#define GFL_GREY     0x0002
#define GFL_COLORS   0x0004
#define GFL_RGB      0x0010
#define GFL_RGBA     0x0020
#define GFL_BGR      0x0040
#define GFL_ABGR     0x0080
#define GFL_BGRA     0x0100
#define GFL_ARGB     0x0200
#define GFL_CMYK     0x0400

/*
 * OBSOLETE
 */
#define GFL_24BITS           (GFL_RGB | GFL_BGR)
#define GFL_32BITS           (GFL_RGBA | GFL_ABGR | GFL_BGRA | GFL_ARGB | GFL_CMYK)
#define GFL_TRUECOLORS       (GFL_24BITS | GFL_32BITS)

#define GFL_IS24BITS(_a)     ((_a) & GFL_24BITS)
#define GFL_IS32BITS(_a)     ((_a) & GFL_32BITS)
#define GFL_ISTRUECOLORS(_a) ((_a) & GFL_TRUECOLORS)
/*
 * ~OBSOLETE
 */

#undef GFL_ISTRUECOLORS
#undef GFL_24BITS
#undef GFL_32BITS
#define GFL_24BITS   0x1000 /* Only for gflBitmapTypeIsSupportedByIndex or gflBitmapTypeIsSupportedByName */
#define GFL_32BITS   0x2000
#define GFL_48BITS   0x4000
#define GFL_64BITS   0x8000

#define GFL_ISTRUECOLORS(_a) ((_a) & (GFL_RGB | GFL_BGR | GFL_RGBA | GFL_ABGR | GFL_BGRA | GFL_ARGB | GFL_CMYK))

typedef GFL_UINT16 GFL_BITMAP_TYPE; 

/*
 *  BITMAP struct
 */
typedef struct {
		GFL_UINT8 Red[256]; 
		GFL_UINT8 Green[256]; 
		GFL_UINT8 Blue[256]; 
		GFL_UINT8 Alpha[256]; 
	} GFL_COLORMAP; 

typedef struct {
		GFL_UINT16 Red, Green, Blue, Alpha; 
	} GFL_COLOR; 

/*
 *  BITMAP struct
 */
typedef struct {
		GFL_BITMAP_TYPE  Type; 
		GFL_ORIGIN       Origin; 
		GFL_INT32        Width; 
		GFL_INT32        Height; 
		GFL_UINT32       BytesPerLine; 
		GFL_INT16        LinePadding; 
		GFL_UINT16       BitsPerComponent;  /* 1, 8, 10, 12, 16 */
		GFL_UINT16       ComponentsPerPixel;/* 1, 3, 4  */
		GFL_UINT16       BytesPerPixel;     /* Only valid for 8 or more bits */
		GFL_UINT16       Xdpi; 
		GFL_UINT16       Ydpi; 
		GFL_INT16        TransparentIndex;  /* -1 if not used */
		GFL_INT16	       Reserved; 
		GFL_INT32        ColorUsed; 
		GFL_COLORMAP*    ColorMap; 
		GFL_UINT8*       Data; 
		char*            Comment; 
		void*            MetaData; 

		GFL_INT32        XOffset; 
		GFL_INT32        YOffset; 
		char*            Name; 
	} GFL_BITMAP; 

/*
 *    Channels Order
 */
#define GFL_CORDER_INTERLEAVED 0
#define GFL_CORDER_SEQUENTIAL  1
#define GFL_CORDER_SEPARATE    2

typedef GFL_UINT16 GFL_CORDER; 

/*
 *    Channels Type
 */
#define GFL_CTYPE_GREYSCALE    0
#define GFL_CTYPE_RGB          1
#define GFL_CTYPE_BGR          2
#define GFL_CTYPE_RGBA         3
#define GFL_CTYPE_ABGR         4
#define GFL_CTYPE_CMY          5
#define GFL_CTYPE_CMYK         6

typedef GFL_UINT16 GFL_CTYPE; 

/*
 *    Lut Type (For DPX/Cineon)
 */
#define GFL_LUT_TO8BITS       1
#define GFL_LUT_TO10BITS      2
#define GFL_LUT_TO12BITS      3
#define GFL_LUT_TO16BITS      4

typedef GFL_UINT16 GFL_LUT_TYPE; 

/*
 *  Callbacks
 */
typedef void* (GFLAPI *GFL_ALLOC_CALLBACK)( size_t size, void* user_parms ); 
typedef void* (GFLAPI *GFL_REALLOC_CALLBACK)( void* ptr, size_t new_size, void* user_parms ); 
typedef void (GFLAPI *GFL_FREE_CALLBACK)( void* ptr, void* user_parms ); 

typedef void * GFL_HANDLE; 

typedef GFL_UINT32 (GFLAPI *GFL_READ_CALLBACK)(GFL_HANDLE handle, void* buffer, GFL_UINT32 size); 
typedef GFL_UINT32 (GFLAPI *GFL_TELL_CALLBACK)(GFL_HANDLE handle); 
typedef GFL_UINT32 (GFLAPI *GFL_SEEK_CALLBACK)(GFL_HANDLE handle, GFL_INT32 offset, GFL_INT32 origin); 
typedef GFL_UINT32 (GFLAPI *GFL_WRITE_CALLBACK)(GFL_HANDLE handle, const void* buffer, GFL_UINT32 size); 

typedef void* (GFLAPI *GFL_ALLOCATEBITMAP_CALLBACK)(GFL_INT32 width, GFL_INT32 height, GFL_INT32 number_component, GFL_INT32 bits_per_component, GFL_INT32 padding, GFL_INT32 bytes_per_line, void * user_params); /* Be careful when using this callback */
typedef void (GFLAPI *GFL_PROGRESS_CALLBACK)(GFL_INT32 percent, void* user_params); 
typedef GFL_BOOL (GFLAPI *GFL_WANTCANCEL_CALLBACK)(void* user_params); 

typedef GFL_ERROR (GFLAPI *GFL_VIRTUAL_SAVE_CALLBACK)(void** pBuffer, GFL_INT32 line, GFL_INT32 nbline, void* user_params); 

typedef struct {
		GFL_BITMAP_TYPE  Type; 
		GFL_ORIGIN       Origin; 
		GFL_INT32        Width; 
		GFL_INT32        Height; 
		GFL_UINT32       BytesPerLine; 
		GFL_INT16        LinePadding; 
		GFL_UINT16       BitsPerComponent; 
		GFL_UINT16       ComponentsPerPixel;
		GFL_UINT16       BytesPerPixel; 
	} GFL_LOAD_INFO_CB; 
typedef GFL_ERROR (GFLAPI *GFL_VIRTUAL_LOAD_CALLBACK)(const void* pBuffer, GFL_INT32 line, GFL_INT32 nbline, const GFL_LOAD_INFO_CB* info, void* user_params); 

/*
 *  LOAD_PARAMS Flags
 */
#define GFL_LOAD_SKIP_ALPHA					      0x00000001 /* Alpha not loaded (32bits only)                     */
#define GFL_LOAD_IGNORE_READ_ERROR	      0x00000002
#define GFL_LOAD_BY_EXTENSION_ONLY        0x00000004 /* Use only extension to recognize format. Faster     */
#define GFL_LOAD_READ_ALL_COMMENT         0x00000008 /* Read Comment in GFL_FILE_DESCRIPTION               */
#define GFL_LOAD_FORCE_COLOR_MODEL        0x00000010 /* Force to load picture in the ColorModel            */
#define GFL_LOAD_PREVIEW_NO_CANVAS_RESIZE 0x00000020 /* With gflLoadPreview, width & height are the maximum box */
#define GFL_LOAD_BINARY_AS_GREY           0x00000040 /* Load Black&White file in greyscale                 */
#define GFL_LOAD_ORIGINAL_COLORMODEL      0x00000080 /* If the colormodel is CMYK, keep it                 */
#define GFL_LOAD_ONLY_FIRST_FRAME         0x00000100 /* No search to check if file is multi-frame          */
#define GFL_LOAD_ORIGINAL_DEPTH           0x00000200 /* In the case of 10/16 bits per component            */
#define GFL_LOAD_METADATA                 0x00000400 /* Read all metadata                                  */
#define GFL_LOAD_COMMENT                  0x00000800 /* Read comment                                       */
#define GFL_LOAD_HIGH_QUALITY_THUMBNAIL   0x00001000 /* gflLoadThumbnail                                   */
#define GFL_LOAD_EMBEDDED_THUMBNAIL       0x00002000 /* gflLoadThumbnail                                   */
#define GFL_LOAD_ORIENTED_THUMBNAIL       0x00004000 /* gflLoadThumbnail                                   */
#define GFL_LOAD_ORIGINAL_EMBEDDED_THUMBNAIL 0x00008000 /* gflLoadThumbnail                                   */
#define GFL_LOAD_ORIENTED									0x00008000 
/*
 *  GFL_LOAD_CALLBACKS struct
 */
typedef struct {
		GFL_READ_CALLBACK  Read; 
		GFL_TELL_CALLBACK  Tell; 
		GFL_SEEK_CALLBACK  Seek; 

		GFL_ALLOCATEBITMAP_CALLBACK AllocateBitmap; /* Global or not???? */
		void * AllocateBitmapParams; 

		GFL_PROGRESS_CALLBACK Progress; 
		void * ProgressParams; 

		GFL_WANTCANCEL_CALLBACK WantCancel; 
		void * WantCancelParams; 

		GFL_VIRTUAL_LOAD_CALLBACK SetLine; 
		void* SetLineParams; 

	} GFL_LOAD_CALLBACKS; 

/*
 *  LOAD_PARAMS struct
 */
typedef struct {
		GFL_UINT32       Flags; 
		GFL_INT32        FormatIndex; /* -1 for automatic recognition */
    GFL_INT32        ImageWanted; /* for multi-page or animated file */
		GFL_ORIGIN       Origin;      /* default: GFL_TOP_LEFT   */
		GFL_BITMAP_TYPE  ColorModel;  /* Only for 24/32 bits picture, GFL_RGB/GFL_RGBA (default), GFL_BGR/GFL_ABGR, GFL_BGRA, GFL_ARGB */
		GFL_UINT32       LinePadding; /* 1 (default), 2, 4, .... */
		
		GFL_UINT8        DefaultAlpha; /* Used if alpha doesn't exist in original file & ColorModel=RGBA/BGRA/ABGR/ARGB */

		/*
		 *
		 */
		GFL_UINT8        PsdNoAlphaForNonLayer; 
		GFL_UINT8        PngComposeWithAlpha; 
		GFL_UINT8        WMFHighResolution; 

		/*
		 * RAW/YUV only
		 */
		GFL_INT32        Width; 
		GFL_INT32        Height; 
		GFL_UINT32       Offset; 

		/*
		 * RAW only
		 */
		GFL_CORDER       ChannelOrder; 
		GFL_CTYPE        ChannelType; 
		
		/*
		 * PCD only
		 */
		GFL_UINT16       PcdBase; /* PCD -> 2:768x576, 1:384x288, 0:192x144 */

		/*
		 * EPS/PS/AI/PDF only
		 */
		GFL_UINT16       EpsDpi; 
		GFL_INT32        EpsWidth; 
		GFL_INT32        EpsHeight; 
		
		/*
		 * DPX/Cineon only
		 */
		GFL_LUT_TYPE     LutType; /* GFL_LUT_TO8BITS, GFL_LUT_TO10BITS, GFL_LUT_TO12BITS, GFL_LUT_TO16BITS */
		GFL_UINT16       Reserved3; 
		GFL_UINT16     * LutData; /* RRRR.../GGGG..../BBBB.....*/
		const char     * LutFilename; 
    
		/* 
		 * Camera RAW only
		 */
		GFL_UINT8        CameraRawUseAutomaticBalance; 
		GFL_UINT8        CameraRawUseCameraBalance; 
		GFL_UINT8        CameraRawHighlight; 
		GFL_UINT8        CameraRawAutoBright; 
		float            CameraRawGamma; 
		float            CameraRawBrightness; 
		float            CameraRawRedScaling; 
		float            CameraRawBlueScaling; 
		//float            CameraRawFilterDomain; 
		//float            CameraRawFilterRange; 
		
		GFL_LOAD_CALLBACKS Callbacks; 
		
		void           * UserParams; 
	} GFL_LOAD_PARAMS; 

/*
 *  SAVE_PARAMS Flags
 */
#define GFL_SAVE_REPLACE_EXTENSION 0x0001
#define GFL_SAVE_WANT_FILENAME     0x0002
#define GFL_SAVE_ANYWAY            0x0004
#define GFL_SAVE_ICC_PROFILE       0x0008	/* Currently only available for jpeg */

/*
 *  SAVE_PARAMS struct
 */
typedef struct {
		GFL_UINT32       Flags; 
		GFL_INT32        FormatIndex; 

		GFL_COMPRESSION  Compression; 
		GFL_INT16        Quality;           /* JPEG/Wic/Fpx  */
		GFL_INT16        CompressionLevel;  /* PNG           */
		GFL_BOOL         Interlaced;        /* GIF           */
		GFL_BOOL         Progressive;       /* JPEG          */
		GFL_BOOL         OptimizeHuffmanTable; /* JPEG       */
		GFL_BOOL         InAscii;           /* PPM           */

		/*
		 * DPX/Cineon only
		 */
		GFL_LUT_TYPE     LutType; /* GFL_LUT_TO8BITS, GFL_LUT_TO10BITS, GFL_LUT_TO12BITS, GFL_LUT_TO16BITS */
#define GFL_BYTE_ORDER_DEFAULT	0
#define GFL_BYTE_ORDER_LSBF			1
#define GFL_BYTE_ORDER_MSBF			2
		GFL_UINT8        DpxByteOrder; 
		GFL_UINT8        CompressRatio; /* JPEG2000 */
		GFL_UINT32       MaxFileSize;   /* JPEG2000 */

		GFL_UINT16     * LutData; /* RRRR.../GGGG..../BBBB.....*/
		const char     * LutFilename; 

		/*
		 * For RAW/YUV
		 */
		GFL_UINT32       Offset; 
		GFL_CORDER       ChannelOrder; 
		GFL_CTYPE        ChannelType; 

		struct {
				GFL_WRITE_CALLBACK Write; 
				GFL_TELL_CALLBACK  Tell;  
				GFL_SEEK_CALLBACK  Seek;  

				GFL_VIRTUAL_SAVE_CALLBACK GetLine; 
				void* GetLineParams; 
			} Callbacks; 

		void           * UserParams; 

	} GFL_SAVE_PARAMS; 

/*
 *  Color model
 */
#define GFL_CM_RGB                0
#define GFL_CM_GREY               1
#define GFL_CM_CMY                2
#define GFL_CM_CMYK               3
#define GFL_CM_YCBCR              4
#define GFL_CM_YUV16              5
#define GFL_CM_LAB                6
#define GFL_CM_LOGLUV             7
#define GFL_CM_LOGL               8

typedef GFL_UINT16 GFL_COLORMODEL; 

/*
 *  FILE_INFORMATION struct
 */
typedef struct {
		GFL_BITMAP_TYPE  Type;   /* Not used */
		GFL_ORIGIN       Origin; 
		GFL_INT32        Width; 
		GFL_INT32        Height; 
		GFL_INT32        FormatIndex; 
		char             FormatName[8]; 
		char             Description[64]; 
		GFL_UINT16       Xdpi; 
		GFL_UINT16       Ydpi; 
		GFL_UINT16       BitsPerComponent;  /* 1, 8, 10, 12, 16 */
		GFL_UINT16       ComponentsPerPixel;/* 1, 3, 4  */
		GFL_INT32        NumberOfImages; 
		GFL_UINT32       FileSize; 
		GFL_COLORMODEL   ColorModel; 
		GFL_COMPRESSION  Compression; 
		char             CompressionDescription[64]; 
		GFL_INT32        XOffset; 
		GFL_INT32        YOffset; 

		void*            ExtraInfos; 
	} GFL_FILE_INFORMATION; 

#define GFL_READ	0x01
#define GFL_WRITE	0x02

/*
 *  FORMAT_INFORMATION struct
 */
typedef struct {
		GFL_INT32        Index; 
		char             Name[8]; 
		char             Description[64]; 
    GFL_UINT32       Status; 
		GFL_UINT32       NumberOfExtension; 
		char             Extension[16][8]; 
	} GFL_FORMAT_INFORMATION; 

/* 
 *  IPTC
 */
typedef struct {
		GFL_UINT32 Id; 
		const char * Name; 
		const char * Value; 
	} GFL_IPTC_ENTRY; 

typedef struct {
		GFL_UINT32       NumberOfItems; 
		GFL_IPTC_ENTRY * ItemsList; 
	} GFL_IPTC_DATA; 

#define GFL_IPTC_BYLINE										0x50
#define GFL_IPTC_BYLINETITLE							0x55
#define GFL_IPTC_CREDITS 									0x6e
#define GFL_IPTC_SOURCE 									0x73
#define GFL_IPTC_CAPTIONWRITER 						0x7a
#define GFL_IPTC_CAPTION 									0x78
#define GFL_IPTC_HEADLINE 								0x69
#define GFL_IPTC_SPECIALINSTRUCTIONS 			0x28
#define GFL_IPTC_OBJECTNAME 							0x05
#define GFL_IPTC_DATECREATED 							0x37
#define GFL_IPTC_RELEASEDATE 							0x1e
#define GFL_IPTC_TIMECREATED 							0x3c
#define GFL_IPTC_RELEASETIME 							0x23
#define GFL_IPTC_CITY 										0x5a
#define GFL_IPTC_STATE 										0x5f
#define GFL_IPTC_COUNTRY 									0x65
#define GFL_IPTC_COUNTRYCODE 							0x64
#define GFL_IPTC_SUBLOCATION 							0x5c
#define GFL_IPTC_ORIGINALTRREF 						0x67
#define GFL_IPTC_CATEGORY 								0x0f
#define GFL_IPTC_COPYRIGHT 								0x74
#define GFL_IPTC_EDITSTATUS 							0x07
#define GFL_IPTC_PRIORITY 								0x0a
#define GFL_IPTC_OBJECTCYCLE 							0x4b
#define GFL_IPTC_JOBID 										0x16
#define GFL_IPTC_PROGRAM 									0x41
#define GFL_IPTC_KEYWORDS									0x19
#define GFL_IPTC_SUPCATEGORIES						0x14
#define GFL_IPTC_CONTENT_LOCATION         0x1b
#define GFL_IPTC_PROGRAM_VERSION          0x46
#define GFL_IPTC_CONTACT                  0x76

/* 
 *  EXIF
 */
#define GFL_EXIF_IFD_0									0x0001
#define GFL_EXIF_MAIN_IFD								0x0002
#define GFL_EXIF_INTEROPERABILITY_IFD		0x0004
#define GFL_EXIF_IFD_THUMBNAIL					0x0008
#define GFL_EXIF_GPS_IFD								0x0010
#define GFL_EXIF_MAKERNOTE_IFD					0x0020

#define GFL_EXIF_MAKER									0x010F
#define GFL_EXIF_MODEL									0x0110
#define GFL_EXIF_ORIENTATION						0x0112
#define GFL_EXIF_EXPOSURETIME						0x829A
#define GFL_EXIF_FNUMBER								0x829D
#define GFL_EXIF_DATETIME_ORIGINAL			0x9003
#define GFL_EXIF_SHUTTERSPEED						0x9201	
#define GFL_EXIF_APERTURE								0x9202
#define GFL_EXIF_MAXAPERTURE						0x9205
#define GFL_EXIF_FOCALLENGTH						0x920A

typedef struct {
		GFL_UINT32 Flag; /* EXIF_...IFD */
		GFL_UINT32 Tag; 
		const char * Name; 
		const char * Value; 
	} GFL_EXIF_ENTRY; 

typedef struct {
		GFL_UINT32       NumberOfItems; 
		GFL_EXIF_ENTRY * ItemsList; 
	} GFL_EXIF_DATA; 

/*
 * For advanced developer only!!!
 */
#define GFL_EXIF_BYTE       1 
#define GFL_EXIF_STRING     2
#define GFL_EXIF_USHORT     3
#define GFL_EXIF_ULONG      4
#define GFL_EXIF_URATIONAL  5
#define GFL_EXIF_SBYTE      6
#define GFL_EXIF_UNDEFINED  7
#define GFL_EXIF_SSHORT     8
#define GFL_EXIF_SLONG      9
#define GFL_EXIF_SRATIONAL 10
#define GFL_EXIF_SINGLEF   11
#define GFL_EXIF_DOUBLE    12

typedef struct _gfl_exif_entryex {
		GFL_UINT16 Tag; 
		GFL_UINT16 Format; 
		GFL_INT32 Ifd; 
		GFL_INT32 NumberOfComponents; 
		GFL_UINT32 Value; 
		GFL_INT32 DataLength; 
		char* Data; 

		struct _gfl_exif_entryex* Next; 
	} GFL_EXIF_ENTRYEX; 

typedef struct {
		GFL_EXIF_ENTRYEX* Root; 
		GFL_INT32	UseMsbf; 
	} GFL_EXIF_DATAEX; 

/*
 *  Functions
 */

extern GFLEXTERN void* GFLAPI gflMemoryAlloc( GFL_UINT32 size ); 

extern GFLEXTERN void* GFLAPI gflMemoryRealloc( void* ptr, GFL_UINT32 size ); 

extern GFLEXTERN void GFLAPI gflMemoryFree( void* ptr ); 

/*
 * ~~
 */

extern GFLEXTERN const char* GFLAPI gflGetVersion( void ); 

extern GFLEXTERN const char* GFLAPI gflGetVersionOfLibformat( void ); 

/*
 * ~~
 */

extern GFLEXTERN GFL_ERROR GFLAPI gflLibraryInit( void ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflLibraryInitEx( GFL_ALLOC_CALLBACK alloc_callback, GFL_REALLOC_CALLBACK realloc_callback, GFL_FREE_CALLBACK free_callback, void * user_parms ); 

extern GFLEXTERN void GFLAPI gflLibraryExit( void ); 

extern GFLEXTERN void GFLAPI gflEnableLZW( GFL_BOOL ); /* If you have the Unisys license */

extern GFLEXTERN void GFLAPI gflSetPluginsPathname( const char* ); 

/*
 * ~~
 */

extern GFLEXTERN GFL_INT32 GFLAPI gflGetNumberOfFormat( void ); 

extern GFLEXTERN GFL_INT32 GFLAPI gflGetFormatIndexByName( const char* name ); 

extern GFLEXTERN const char* GFLAPI gflGetFormatNameByIndex( GFL_INT32 index ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflFormatIsSupported( const char* name ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflFormatIsWritableByIndex( GFL_INT32 index ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflFormatIsWritableByName( const char* name ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflFormatIsReadableByIndex( GFL_INT32 index ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflFormatIsReadableByName( const char* name ); 

extern GFLEXTERN const char* GFLAPI gflGetDefaultFormatSuffixByIndex( GFL_INT32 index ); 

extern GFLEXTERN const char* GFLAPI gflGetDefaultFormatSuffixByName( const char* name ); 

extern GFLEXTERN const char* GFLAPI gflGetFormatDescriptionByIndex( GFL_INT32 index ); 

extern GFLEXTERN const char* GFLAPI gflGetFormatDescriptionByName( const char* name ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflGetFormatInformationByIndex( GFL_INT32 index, GFL_FORMAT_INFORMATION* info ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflGetFormatInformationByName( const char* name, GFL_FORMAT_INFORMATION* info ); 

/*
 * ~~
 */

#define GFL_SAVE_PARAMS_QUALITY           0  /* 0<=quality<=100 */
#define GFL_SAVE_PARAMS_COMPRESSION_LEVEL 1  /* 0<=level<=9     */
#define GFL_SAVE_PARAMS_INTERLACED        2
#define GFL_SAVE_PARAMS_PROGRESSIVE       3
#define GFL_SAVE_PARAMS_OPTIMIZE_HUFFMAN  4
#define GFL_SAVE_PARAMS_IN_ASCII          5
#define GFL_SAVE_PARAMS_LUT               6

typedef GFL_UINT32 GFL_SAVE_PARAMS_TYPE; 

extern GFLEXTERN GFL_BOOL GFLAPI gflSaveParamsIsSupportedByIndex( GFL_INT32 index, GFL_SAVE_PARAMS_TYPE type ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflSaveParamsIsSupportedByName( const char* name, GFL_SAVE_PARAMS_TYPE type ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflCompressionIsSupportedByIndex( GFL_INT32 index, GFL_COMPRESSION comp ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflCompressionIsSupportedByName( const char* name, GFL_COMPRESSION comp ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapIsSupportedByIndex( GFL_INT32 index, const GFL_BITMAP* bitmap ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapIsSupportedByName( const char* name, const GFL_BITMAP* bitmap ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapTypeIsSupportedByIndex( GFL_INT32 index, GFL_BITMAP_TYPE type, GFL_UINT16 bits_per_component ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapTypeIsSupportedByName( const char* name, GFL_BITMAP_TYPE type, GFL_UINT16 bits_per_component ); 

/*
 * ~~
 */

extern GFLEXTERN const char * GFLAPI gflGetErrorString( GFL_ERROR error ); 

extern GFLEXTERN const char * GFLAPI gflGetLabelForColorModel( GFL_COLORMODEL color_model ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflGetFileInformation( const char* filename, GFL_INT32 index, GFL_FILE_INFORMATION *info ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflGetFileInformationEx( const char* filename, GFL_INT32 index, GFL_FILE_INFORMATION *info, GFL_UINT32 load_infos ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflGetFileInformationFromHandle( GFL_HANDLE handle, GFL_INT32 index, const GFL_LOAD_CALLBACKS* callbacks, GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflGetFileInformationFromMemory( const GFL_UINT8* data, GFL_UINT32 data_length, GFL_INT32 index, GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN void GFLAPI gflFreeFileInformation( GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN void GFLAPI gflGetDefaultLoadParams( GFL_LOAD_PARAMS* params ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflLoadBitmap( const char* filename, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflLoadBitmapFromHandle( GFL_HANDLE handle, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflLoadBitmapFromMemory( const GFL_UINT8* data, GFL_UINT32 data_length, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN void GFLAPI gflGetDefaultThumbnailParams( GFL_LOAD_PARAMS* params ); 

#define gflLoadPreview gflLoadThumbnail
#define gflLoadPreviewFromHandle gflLoadThumbnailFromHandle
#define gflGetDefaultPreviewParams gflGetDefaultThumbnailParams

extern GFLEXTERN GFL_ERROR GFLAPI gflLoadThumbnail( const char* filename, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflLoadThumbnailFromHandle( GFL_HANDLE handle, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION *info ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflLoadThumbnailFromMemory( const GFL_UINT8 * data, GFL_UINT32 data_length, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN void GFLAPI gflGetDefaultSaveParams( GFL_SAVE_PARAMS* params ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflSaveBitmap( char* filename, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS* params ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflSaveBitmapIntoHandle( GFL_HANDLE handle, const GFL_BITMAP* bitmap, const GFL_SAVE_PARAMS* params ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflSaveBitmapIntoMemory( GFL_UINT8** data, GFL_UINT32* data_length, const GFL_BITMAP* bitmap, const GFL_SAVE_PARAMS* params ); 

/* UNICODE support */
#ifdef WIN32

extern GFLEXTERN void GFLAPI gflSetPluginsPathnameW( const wchar_t* ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflGetFileInformationW( const wchar_t* filename, GFL_INT32 index, GFL_FILE_INFORMATION *info ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflLoadBitmapW( const wchar_t* filename, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflLoadThumbnailW( const wchar_t* filename, GFL_INT32 width, GFL_INT32 height, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflSaveBitmapW( wchar_t* filename, const GFL_BITMAP *bitmap, const GFL_SAVE_PARAMS* params ); 

#endif

/*
 * ~~
 */

typedef void *GFL_FILE_HANDLE; 

extern GFLEXTERN GFL_ERROR GFLAPI gflFileCreate         ( GFL_FILE_HANDLE* handle, const char* filename, GFL_UINT32 image_count, const GFL_SAVE_PARAMS* params ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflFileAddPicture     ( GFL_FILE_HANDLE handle, const GFL_BITMAP *bitmap ); 

extern GFLEXTERN void GFLAPI gflFileClose               ( GFL_FILE_HANDLE handle ); 

/* UNICODE support */
#ifdef WIN32
extern GFLEXTERN GFL_ERROR GFLAPI gflFileCreateW( GFL_FILE_HANDLE* handle, const wchar_t* filename, GFL_UINT32 image_count, const GFL_SAVE_PARAMS* params ); 
#endif

/*
 * ~~
 */

extern GFLEXTERN GFL_BITMAP* GFLAPI gflAllockBitmap( GFL_BITMAP_TYPE type, GFL_INT32 width, GFL_INT32 height, GFL_UINT32 line_padding, const GFL_COLOR*  color ); 

extern GFLEXTERN GFL_BITMAP* GFLAPI gflAllockBitmapEx( GFL_BITMAP_TYPE type, GFL_INT32 width, GFL_INT32 height, GFL_UINT16 bits_per_component, GFL_UINT32 padding, const GFL_COLOR*  color ); 

extern GFLEXTERN void GFLAPI gflFreeBitmap( GFL_BITMAP* bitmap ); 

extern GFLEXTERN void GFLAPI gflFreeBitmapData( GFL_BITMAP* bitmap ); /* bitmap is not freed */

extern GFLEXTERN GFL_BITMAP* GFLAPI gflCloneBitmap( const GFL_BITMAP* bitmap ); 

extern GFLEXTERN void GFLAPI gflBitmapSetName( GFL_BITMAP* bitmap, const char* name ); 

extern GFLEXTERN GFL_INT32 GFLAPI gflGetExtraInfosCount( const GFL_FILE_INFORMATION* info ); 

extern GFLEXTERN void GFLAPI gflGetExtraInfos( const GFL_FILE_INFORMATION* info, GFL_INT32 idx, char** name, char** value ); 

/*
 * ~~ METADATA
 */

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapHasEXIF( const GFL_BITMAP* bitmap ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapHasIPTC( const GFL_BITMAP* bitmap ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapHasICCProfile( const GFL_BITMAP* bitmap ); 

extern GFLEXTERN void GFLAPI gflBitmapRemoveEXIFThumbnail( GFL_BITMAP* bitmap ); 

extern GFLEXTERN void GFLAPI gflBitmapRemoveICCProfile( GFL_BITMAP* bitmap ); 

extern GFLEXTERN void GFLAPI gflBitmapGetICCProfile( const GFL_BITMAP* bitmap, GFL_UINT8** pData, GFL_UINT32* pLength ); /* pData must be freed by gflFreeMemory */

extern GFLEXTERN void GFLAPI gflBitmapCopyICCProfile( const GFL_BITMAP* bitmap_src, GFL_BITMAP* bitmap_dst ); 

extern GFLEXTERN void GFLAPI gflBitmapRemoveMetaData( GFL_BITMAP* bitmap ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflBitmapGetXMP( const GFL_BITMAP* bitmap, GFL_UINT8** pData, GFL_UINT32* pLength ); 

extern GFLEXTERN void GFLAPI gflBitmapSetEXIFThumbnail( GFL_BITMAP* bitmap, const GFL_BITMAP* thumb_bitmap ); 

extern GFL_BOOL GFLAPI gflGetEXIFDPI( const GFL_BITMAP* bitmap, GFL_INT32 * xdpi, GFL_INT32 * ydpi ); 

/*
 * ~~ EXIF function without loading bitmap
 */

extern GFLEXTERN GFL_EXIF_DATA* GFLAPI gflLoadEXIF( const char* filename, GFL_UINT32 flags ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflHasEXIF( const char* filename);

extern GFLEXTERN GFL_EXIF_DATAEX* GFLAPI gflLoadEXIF2( const char* filename, GFL_UINT32 flags );

extern GFLEXTERN GFL_BOOL GFLAPI gflHasIPTC( const char* filename );

extern GFLEXTERN GFL_BOOL GFLAPI gflHasICCProfile( const char* filename );

/*
 * ~~ 
 */

extern GFLEXTERN GFL_IPTC_DATA* GFLAPI gflBitmapGetIPTC( GFL_BITMAP* bitmap ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflBitmapGetIPTCValue( const GFL_BITMAP* bitmap, GFL_UINT32 id, char* value, GFL_INT32 value_length ); 

extern GFLEXTERN GFL_IPTC_DATA* GFLAPI gflNewIPTC( void ); 

extern GFLEXTERN void GFLAPI gflFreeIPTC( GFL_IPTC_DATA* iptc_data ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflSetIPTCValue( GFL_IPTC_DATA* iptc_data, GFL_UINT32 id, const char* value ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflRemoveIPTCValue( GFL_IPTC_DATA* iptc_data, GFL_UINT32 id ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflClearIPTCKeywords( GFL_IPTC_DATA* iptc_data ); 

/*
 * ~~ IPTC function without loading bitmap
 */

extern GFLEXTERN GFL_IPTC_DATA* GFLAPI gflLoadIPTC( const char* filename ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflSaveIPTC( const char* filename, const GFL_IPTC_DATA* iptc_data ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflBitmapSetIPTC( GFL_BITMAP* bitmap, const GFL_IPTC_DATA* iptc_data ); 

/* UNICODE support */
#ifdef WIN32
extern GFLEXTERN GFL_EXIF_DATA* GFLAPI gflLoadEXIFW( const wchar_t* filename, GFL_UINT32 flags ); 
extern GFLEXTERN GFL_IPTC_DATA* GFLAPI gflLoadIPTCW( const wchar_t* filename ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflSaveIPTCW( const wchar_t* filename, const GFL_IPTC_DATA* iptc_data ); 
#endif

/*
 * ~~ 
 */

#define GFL_EXIF_WANT_MAKERNOTES	0x0001
extern GFLEXTERN GFL_EXIF_DATA* GFLAPI gflBitmapGetEXIF( GFL_BITMAP* bitmap, GFL_UINT32 flags ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflBitmapGetEXIFValue( const GFL_BITMAP* bitmap, GFL_UINT32 tag, char* value, GFL_INT32 value_length ); 

extern GFLEXTERN void GFLAPI gflFreeEXIF( GFL_EXIF_DATA* exif_data ); 

extern GFLEXTERN void GFLAPI gflBitmapSetComment( GFL_BITMAP* bitmap, const char* comment ); 

/*
 * For Advanced developer only
 */

extern GFLEXTERN GFL_EXIF_DATAEX* GFLAPI gflBitmapGetEXIF2( const GFL_BITMAP* bitmap ); 

extern GFLEXTERN void GFLAPI gflFreeEXIF2( GFL_EXIF_DATAEX* exif_data ); 

extern GFLEXTERN void GFLAPI gflBitmapSetEXIF2( GFL_BITMAP* bitmap, const GFL_EXIF_DATAEX* exif ); 

extern GFLEXTERN void GFLAPI gflBitmapSetEXIFValueString2( GFL_EXIF_DATAEX* exif, GFL_UINT16 ifd, GFL_UINT16 tag, const char* value ); 

extern GFLEXTERN void GFLAPI gflBitmapSetEXIFValueInt2( GFL_EXIF_DATAEX* exif, GFL_UINT16 ifd, GFL_UINT16 tag, GFL_UINT32 format, GFL_UINT32 value ); 

extern GFLEXTERN void GFLAPI gflBitmapSetEXIFValueRational2( GFL_EXIF_DATAEX* exif, GFL_UINT16 ifd, GFL_UINT16 tag, GFL_UINT32 p, GFL_UINT32 q ); 

extern GFLEXTERN void GFLAPI gflBitmapSetEXIFValueRationalArray2( GFL_EXIF_DATAEX* exif, GFL_UINT16 ifd, GFL_UINT16 tag, const GFL_UINT32* pq, GFL_INT32 count ); 

/*
 * ~~ COMMENT function without loading bitmap
 */

extern GFLEXTERN GFL_ERROR GFLAPI gflJPEGGetComment( const char* filename, char* comment, GFL_INT32 max_size ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflJPEGSetComment( const char* filename, const char* comment ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflPNGGetComment( const char* filename, char* comment, GFL_INT32 max_size ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflPNGSetComment( const char* filename, const char* comment ); 

/* UNICODE */
#ifdef WIN32
extern GFLEXTERN GFL_ERROR GFLAPI gflJPEGGetCommentW( const wchar_t* filename, char* comment, GFL_INT32 max_size ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflJPEGSetCommentW( const wchar_t* filename, const char* comment ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflPNGGetCommentW( const wchar_t* filename, char* comment, GFL_INT32 max_size ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflPNGSetCommentW( const wchar_t* filename, const char* comment ); 
#endif

/*
 * ~~ For DPX LUT
 */

extern GFLEXTERN GFL_BOOL GFLAPI gflIsLutFile( const char* filename ); 

extern GFLEXTERN GFL_BOOL GFLAPI gflIsCompatibleLutFile( const char* filename, const GFL_INT32 components_per_pixel, const GFL_INT32 bits_per_component, GFL_LUT_TYPE* lut_type ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflApplyLutFile( GFL_BITMAP* bitmap_src, GFL_BITMAP** bitmap_dst, const char* filename, GFL_LUT_TYPE lut_type ); 

/*
 * ~~
 */
#define GFL_RESIZE_QUICK     0
#define GFL_RESIZE_BILINEAR  1
#define GFL_RESIZE_HERMITE   2
#define GFL_RESIZE_GAUSSIAN  3
#define GFL_RESIZE_BELL      4
#define GFL_RESIZE_BSPLINE   5
#define GFL_RESIZE_MITSHELL  6
#define GFL_RESIZE_LANCZOS   7

extern GFLEXTERN GFL_ERROR GFLAPI gflResize( GFL_BITMAP* src, GFL_BITMAP** dst, GFL_INT32 width, GFL_INT32 height, GFL_UINT32 method, GFL_UINT32 flags ); 

/*
 * ~~ DOESN'T WORKS YET WITH MORE THAN 8BITS PER COMPONENT!
 */

#define GFL_MODE_TO_BINARY         1
#define GFL_MODE_TO_4GREY          2
#define GFL_MODE_TO_8GREY          3
#define GFL_MODE_TO_16GREY         4
#define GFL_MODE_TO_32GREY         5
#define GFL_MODE_TO_64GREY         6
#define GFL_MODE_TO_128GREY        7
#define GFL_MODE_TO_216GREY        8
#define GFL_MODE_TO_256GREY        9
#define GFL_MODE_TO_8COLORS        12
#define GFL_MODE_TO_16COLORS       13
#define GFL_MODE_TO_32COLORS       14
#define GFL_MODE_TO_64COLORS       15
#define GFL_MODE_TO_128COLORS      16
#define GFL_MODE_TO_216COLORS      17
#define GFL_MODE_TO_256COLORS      18
#define GFL_MODE_TO_RGB            19
#define GFL_MODE_TO_RGBA           20
#define GFL_MODE_TO_BGR            21
#define GFL_MODE_TO_ABGR           22
#define GFL_MODE_TO_BGRA           23
#define GFL_MODE_TO_ARGB           24

#define GFL_MODE_TO_TRUE_COLORS    GFL_MODE_TO_RGB

typedef GFL_UINT16 GFL_MODE; 

#define GFL_MODE_NO_DITHER         0
#define GFL_MODE_PATTERN_DITHER    1
#define GFL_MODE_HALTONE45_DITHER  2  /* Only with GFL_MODE_TO_BINARY */
#define GFL_MODE_HALTONE90_DITHER  3  /* Only with GFL_MODE_TO_BINARY */
#define GFL_MODE_ADAPTIVE          4
#define GFL_MODE_FLOYD_STEINBERG   5  /* Only with GFL_MODE_TO_BINARY */

typedef GFL_UINT16 GFL_MODE_PARAMS; 

extern GFLEXTERN GFL_ERROR GFLAPI gflChangeColorDepth( GFL_BITMAP* src, GFL_BITMAP** dst, GFL_MODE mode, GFL_MODE_PARAMS params ); 

/*
 * ~~
 */

#define gflGetBitmapPtr( _bitmap, _y ) \
	((_bitmap)->Data + (_y) * (_bitmap)->BytesPerLine)

extern GFLEXTERN GFL_ERROR GFLAPI gflGetColorAt( const GFL_BITMAP* src, GFL_INT32 x, GFL_INT32 y, GFL_COLOR* color ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflSetColorAt( GFL_BITMAP* src, GFL_INT32 x, GFL_INT32 y, const GFL_COLOR* color ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflGetColorAtEx( const GFL_BITMAP* src, GFL_UINT8* ptr, GFL_COLOR* color ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflSetColorAtEx( GFL_BITMAP* src, GFL_UINT8* ptr, const GFL_COLOR* color ); 

/*
 * ~~
 */

typedef struct {
	GFL_INT32 x, y, w, h; 
} GFL_RECT; 

typedef struct {
		GFL_INT32 x, y; 
	} GFL_POINT; 

extern GFLEXTERN GFL_ERROR GFLAPI gflFlipVertical    ( GFL_BITMAP* src, GFL_BITMAP** dst ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflFlipHorizontal  ( GFL_BITMAP* src, GFL_BITMAP** dst ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflCrop            ( GFL_BITMAP* src, GFL_BITMAP** dst, const GFL_RECT* rect ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflAutoCrop        ( GFL_BITMAP* src, GFL_BITMAP** dst, const GFL_COLOR *color, GFL_INT32 tolerance ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflAutoCrop2       ( GFL_BITMAP* src, GFL_BITMAP** dst, const GFL_COLOR *color, GFL_INT32 tolerance ); 

/*
 * ~~
 */

#define GFL_CANVASRESIZE_TOPLEFT     0
#define GFL_CANVASRESIZE_TOP         1
#define GFL_CANVASRESIZE_TOPRIGHT    2
#define GFL_CANVASRESIZE_LEFT        3
#define GFL_CANVASRESIZE_CENTER      4
#define GFL_CANVASRESIZE_RIGHT       5
#define GFL_CANVASRESIZE_BOTTOMLEFT  6
#define GFL_CANVASRESIZE_BOTTOM      7
#define GFL_CANVASRESIZE_BOTTOMRIGHT 8

typedef GFL_UINT32 GFL_CANVASRESIZE; 

extern GFLEXTERN GFL_ERROR GFLAPI gflResizeCanvas    ( GFL_BITMAP* src, GFL_BITMAP** dst, GFL_INT32 width, GFL_INT32 height, GFL_CANVASRESIZE mode, const GFL_COLOR* color ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflScaleToGrey     ( GFL_BITMAP* src, GFL_BITMAP** dst, GFL_INT32 width, GFL_INT32 height ); 

/*
 * ~~
 */

extern GFLEXTERN GFL_ERROR GFLAPI gflRotate          ( GFL_BITMAP* src, GFL_BITMAP** dst, GFL_INT32 angle, const GFL_COLOR *background_color ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflRotateFine      ( GFL_BITMAP* src, GFL_BITMAP** dst, double angle, const GFL_COLOR *background_color ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflReplaceColor    ( GFL_BITMAP* src, GFL_BITMAP** dst, const GFL_COLOR *color, const GFL_COLOR *new_color, GFL_INT32 tolerance ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflBitblt          ( const GFL_BITMAP* src, const GFL_RECT *rect, GFL_BITMAP *dst, GFL_INT32 x_dest, GFL_INT32 y_dest ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflBitbltEx        ( const GFL_BITMAP* src, const GFL_RECT *rect, GFL_BITMAP *dst, GFL_INT32 x_dest, GFL_INT32 y_dest ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflMerge           ( GFL_BITMAP const *src[], const GFL_POINT origin[], const GFL_UINT32 opacity[], GFL_INT32 num_bitmap, GFL_BITMAP** dst ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflCombineAlpha    ( GFL_BITMAP* src, GFL_BITMAP** dst, const GFL_COLOR* color ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflSetTransparentColor( GFL_BITMAP* src, GFL_BITMAP** dst, const GFL_COLOR * mask_color, const GFL_COLOR* back_color ); 

/*
 * ~
 */

extern GFLEXTERN GFL_ERROR GFLAPI gflSaveBitmapBegin			( void** ptr, const char* filename, const GFL_BITMAP*, const GFL_SAVE_PARAMS* ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflSaveBitmapEnd				( void* _ptr ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflSaveBitmapWriteLine	( void* _ptr, GFL_UINT8* buffer ); 

extern GFLEXTERN GFL_ERROR GFLAPI gflLoadBitmapBegin			( void** ptr, const char* filename, GFL_BITMAP**, const GFL_LOAD_PARAMS*, GFL_FILE_INFORMATION* ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflLoadBitmapEnd				( void* _ptr ); 
extern GFLEXTERN GFL_ERROR GFLAPI gflLoadBitmapReadLine		( void* _ptr, GFL_UINT8* buffer ); 

#if defined( WIN32 ) || defined ( __BORLANDC__ )
	#ifdef __BORLANDC__
		#pragma option -a. 
	#else
		#pragma pack (pop, before_push)
	#endif
#else
#endif

#ifdef __cplusplus
}
#endif

#endif
