//=============================================================================
// File:       config.h
// Contents:   Declarations of macros for configuring the library
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
// $Revision$
// $Date$
//
// Copyright (c) 1996, 1997 Douglas W. Sauder
// All rights reserved.
// 
// IN NO EVENT SHALL DOUGLAS W. SAUDER BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
// THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF DOUGLAS W. SAUDER
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// DOUGLAS W. SAUDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND DOUGLAS W. SAUDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//=============================================================================

#ifndef DW_CONFIG_H
#define DW_CONFIG_H

//-----------------------------------------------------------------------------
// Platform
//
// Make sure that the following lines define either DW_UNIX or DW_WIN32.
//-----------------------------------------------------------------------------

#if defined(_WIN32) || defined(__WIN32__)
#   define DW_WIN32
#endif

#if defined(__unix__) || defined(__unix) || defined(_AIX) || defined(__NetBSD__) || defined(__APPLE__)
#   define DW_UNIX
#endif

//-----------------------------------------------------------------------------
// End of line characters
//
// Uncomment one of the following to indicate whether you want the library to
// use LF or CR LF as the end of line characters.
//
// I strongly recommend using LF ('\n') alone as the end of line character,
// since that is the normal end of line character for C and C++ libraries.
// Then you can do the conversion to and from the CR LF end of line
// characters at the interface to the transport system.
//-----------------------------------------------------------------------------

#define DW_EOL_LF
//#define DW_EOL_CRLF

#if defined(DW_EOL_CRLF)
#   define DW_EOL  "\r\n"
#elif defined(DW_EOL_LF)
#   define DW_EOL  "\n"
#else
#   error "Must define DW_EOL_CRLF, DW_EOL_LF"
#endif

//-----------------------------------------------------------------------------
// C++ namespaces
//
// Uncomment the following line if you want the DwMime namespace to be defined.
// If the namespace is not defined, then enums are specified as part of a
// DwMime class.
//-----------------------------------------------------------------------------

//#define DW_USE_NAMESPACES


//-----------------------------------------------------------------------------
// C++ library string class
//
// Uncomment the following line if you want DwString typedef-ed to the
// ANSI/ISO string class.
//
// *** Important: This option is not working or not fully tested yet ***
//-----------------------------------------------------------------------------

//#define DW_USE_ANSI_STRING


//-----------------------------------------------------------------------------
// bool type
//
// Uncomment the following line if you want DwBool typedef-ed to int instead
// of bool.
//-----------------------------------------------------------------------------

//#define DW_NO_BOOL

#if defined(DW_NO_BOOL)

typedef int            DwBool;
#define DwFalse  0
#define DwTrue   1

#else

typedef bool           DwBool;
#define DwFalse  false
#define DwTrue   true

#endif


//-----------------------------------------------------------------------------
// DLL vs static library
//
// Win32 users: Uncomment out the following line to create a static library
// instead of a DLL.
//-----------------------------------------------------------------------------

// #define DW_NO_DLL

#if defined(DW_WIN32) && !defined(DW_NO_DLL)
#   ifdef DW_IMPLEMENTATION
#      define DW_EXPORT __declspec(dllexport)
#   else
#      define DW_EXPORT __declspec(dllimport)
#   endif
#else
#   define DW_EXPORT
#endif

//-----------------------------------------------------------------------------
// Type definitions
//
// Make sure the following types are accurate for your machine.
//-----------------------------------------------------------------------------

#if defined(__BCPLUSPLUS__) && !defined(__WIN32__)
#   define DW_STD_16_BIT
#endif

#if defined(__alpha) || defined(__sgi)
#   define DW_STD_64_BIT
#endif

#if !defined(DW_STD_16_BIT) && !defined(DW_STD_64_BIT)
#   define DW_STD_32_BIT
#endif

typedef char           DwChar7;  // type for ASCII characters
typedef unsigned char  DwChar8;  // type for 8-bit characters
typedef signed char    DwInt8;   // type for 8-bit signed integers
typedef unsigned char  DwUint8;  // type for 8-bit unsigned integers

#if defined(DW_STD_16_BIT)
typedef int            DwInt16;  // 16-bit signed integers
typedef unsigned int   DwUint16; // 16-bit unsigned integers
typedef long           DwInt32;  // 32-bit signed integers
typedef unsigned long  DwUint32; // 32-bit unsigned integers
#elif defined(DW_STD_32_BIT)
typedef short          DwInt16;
typedef unsigned short DwUint16;
typedef int            DwInt32;
typedef unsigned int   DwUint32;
#elif defined(DW_STD_64_BIT)
typedef short          DwInt16;
typedef unsigned short DwUint16;
typedef int            DwInt32;
typedef unsigned int   DwUint32;
#endif

#endif
