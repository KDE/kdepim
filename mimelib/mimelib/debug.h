//=============================================================================
// File:       dw_debug.h
// Contents:   Macros for debugging
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
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

#ifndef DW_DEBUG_H
#define DW_DEBUG_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#if !defined (DW_DEBUG_VERSION) && !defined (DW_DEVELOPMENT_VERSION)
#define NDEBUG
#endif

#if defined (DW_DEBUG_VERSION)
#define DBG_STMT(x) x;
#else
#define DBG_STMT(x) ;
#endif

#if defined (DW_DEBUG_VERSION) || defined (DW_DEVELOPMENT_VERSION)
#define DEV_STMT(x) x;
#else
#define DEV_STMT(x) ;
#endif

#include <assert.h>

#define ASSERT assert

#endif

