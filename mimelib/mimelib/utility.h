//=============================================================================
// File:       utility.h
// Contents:   Declarations of utility functions for MIME++
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

#ifndef DW_UTILITY_H
#define DW_UTILITY_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

class DwString;


void DW_EXPORT DwInitialize();
void DW_EXPORT DwFinalize();
int  DW_EXPORT DwCteStrToEnum(const DwString& aStr);
void DW_EXPORT DwCteEnumToStr(int aEnum, DwString& aStr);
int  DW_EXPORT DwTypeStrToEnum(const DwString& aStr);
void DW_EXPORT DwTypeEnumToStr(int aEnum, DwString& aStr);
int  DW_EXPORT DwSubtypeStrToEnum(const DwString& aStr);
void DW_EXPORT DwSubtypeEnumToStr(int aEnum, DwString& aStr);
int  DW_EXPORT DwToCrLfEol(const DwString& aSrcStr, DwString& aDestStr);
int  DW_EXPORT DwToLfEol(const DwString& aSrcStr, DwString& aDestStr);
int  DW_EXPORT DwToCrEol(const DwString& aSrcStr, DwString& aDestStr);
int  DW_EXPORT DwToLocalEol(const DwString& aSrcStr, DwString& aDestStr);
int  DW_EXPORT DwEncodeBase64(const DwString& aSrcStr, DwString& aDestStr);
int  DW_EXPORT DwDecodeBase64(const DwString& aSrcStr, DwString& aDestStr);
int  DW_EXPORT DwEncodeQuotedPrintable(const DwString& aSrcStr, DwString& aDestStr);
int  DW_EXPORT DwDecodeQuotedPrintable(const DwString& aSrcStr, DwString& aDestStr);

#endif
